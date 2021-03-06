/*
 * fast-pbkdf2 - Optimal PBKDF2-HMAC calculation
 * Written in 2015 by Joseph Birr-Pixton <jpixton@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
/* librist. Copyright © 2019 SipRadius LLC. All right reserved.
 * Author: Kuldeep Singh Dhaka <kuldeep@madresistor.com>
 * Author: Sergio Ammirata, Ph.D. <sergio@ammirata.net>
 *
 * Changes:
 *  - Removed SHA1 and SHA512 as the functions are not available
 *  - Moved to self hosted SHA256 instead of OpenSSL
 */

#include "fastpbkdf2.h"

#include <assert.h>
#include <string.h>

#if defined(__linux__) || defined(__GLIBC__)
#include <endian.h>
#elif defined(__APPLE__) || defined(__GLIBC__)
#include <machine/endian.h>
#define __LITTLE_ENDIAN __DARWIN_LITTLE_ENDIAN
#define __BYTE_ORDER __DARWIN_BYTE_ORDER
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#define __LITTLE_ENDIAN _LITTLE_ENDIAN
#define __BYTE_ORDER _BYTE_ORDER
#endif /* __MacOS__ */

#include "sha256.h"

/* --- MSVC doesn't support C99 --- */
#ifdef _MSC_VER
#define restrict
#define _Pragma __pragma
#endif

/* --- Common useful things --- */
#define MIN(a, b) ((a) > (b)) ? (b) : (a)

static inline void write32_be(uint32_t n, uint8_t out[4])
{
#if defined(__GNUC__) && __GNUC__ >= 4 && __BYTE_ORDER == __LITTLE_ENDIAN
  *(uint32_t *)(out) = __builtin_bswap32(n);
#else
  out[0] = (n >> 24) & 0xff;
  out[1] = (n >> 16) & 0xff;
  out[2] = (n >> 8) & 0xff;
  out[3] = n & 0xff;
#endif
}

/*
 * NOTE: This function is unused currently, but it is left incase it is needed in the
 *       future
 */

/* static inline void write64_be(uint64_t n, uint8_t out[8])
 * {
 * #if defined(__GNUC__) &&  __GNUC__ >= 4 && __BYTE_ORDER == __LITTLE_ENDIAN
 * 	*(uint64_t *)(out) = __builtin_bswap64(n);
 * #else
 * 	write32_be((n >> 32) & 0xffffffff, out);
 * 	write32_be(n & 0xffffffff, out + 4);
 * #endif
 * }
 */

/* --- Optional OpenMP parallelisation of consecutive blocks --- */
#ifdef WITH_OPENMP
# define OPENMP_PARALLEL_FOR _Pragma("omp parallel for")
#else
# define OPENMP_PARALLEL_FOR
#endif

/* Prepare block (of blocksz bytes) to contain md padding denoting a msg-size
 * message (in bytes).  block has a prefix of used bytes.
 *
 * Message length is expressed in 32 bits (so suitable for sha1, sha256, sha512). */
static inline void md_pad(uint8_t *block, size_t blocksz, size_t used, size_t msg)
{
	memset(block + used, 0, blocksz - used - 4);
	block[used] = 0x80;
	block += blocksz - 4;
	write32_be((uint32_t) (msg * 8), block);
}

/* Internal function/type names for hash-specific things. */
#define HMAC_CTX(_name) HMAC_ ## _name ## _ctx
#define HMAC_INIT(_name) HMAC_ ## _name ## _init
#define HMAC_UPDATE(_name) HMAC_ ## _name ## _update
#define HMAC_FINAL(_name) HMAC_ ## _name ## _final

#define PBKDF2_F(_name) pbkdf2_f_ ## _name
#define PBKDF2(_name) pbkdf2_ ## _name

/* This macro expands to decls for the whole implementation for a given
 * hash function.  Arguments are:
 *
 * _name like 'sha1', added to symbol names
 * _blocksz block size, in bytes
 * _hashsz digest output, in bytes
 * _ctx hash context type
 * _init hash context initialisation function
 *    args: (_ctx *c)
 * _update hash context update function
 *    args: (_ctx *c, const void *data, size_t ndata)
 * _final hash context finish function
 *    args: (void *out, _ctx *c)
 * _xform hash context raw block update function
 *    args: (_ctx *c, const void *data)
 * _xcpy hash context raw copy function (only need copy hash state)
 *    args: (_ctx * restrict out, const _ctx *restrict in)
 * _xtract hash context state extraction
 *    args: args (_ctx *restrict c, uint8_t *restrict out)
 * _xxor hash context xor function (only need xor hash state)
 *    args: (_ctx *restrict out, const _ctx *restrict in)
 *
 * The resulting function is named PBKDF2(_name).
 */
#define DECL_PBKDF2(_name, _blocksz, _hashsz, _ctx,                           \
                    _init, _update, _xform, _final, _xcpy, _xtract, _xxor)    \
  typedef struct {                                                            \
    _ctx inner;                                                               \
    _ctx outer;                                                               \
  } HMAC_CTX(_name);                                                          \
                                                                              \
  static inline void HMAC_INIT(_name)(HMAC_CTX(_name) *ctx,                   \
                                      const uint8_t *key, size_t nkey)        \
  {                                                                           \
    /* Prepare key: */                                                        \
    uint8_t k[_blocksz];                                                      \
                                                                              \
    /* Shorten long keys. */                                                  \
    if (nkey > _blocksz)                                                      \
    {                                                                         \
      _init(&ctx->inner);                                                     \
      _update(&ctx->inner, key, nkey);                                        \
      _final(&ctx->inner, k);                                                 \
                                                                              \
      key = k;                                                                \
      nkey = _hashsz;                                                         \
    }                                                                         \
                                                                              \
    /* Standard doesn't cover case where blocksz < hashsz. */                 \
    assert(nkey <= _blocksz);                                                 \
                                                                              \
    /* Right zero-pad short keys. */                                          \
    if (k != key)                                                             \
      memcpy(k, key, nkey);                                                   \
    if (_blocksz > nkey)                                                      \
      memset(k + nkey, 0, _blocksz - nkey);                                   \
                                                                              \
    /* Start inner hash computation */                                        \
    uint8_t blk_inner[_blocksz];                                              \
    uint8_t blk_outer[_blocksz];                                              \
                                                                              \
                                                                              \
    size_t i = 0;                                                             \
    for (i = 0; i < _blocksz; i++)                                            \
    {                                                                         \
      blk_inner[i] = 0x36 ^ k[i];                                             \
      blk_outer[i] = 0x5c ^ k[i];                                             \
    }                                                                         \
                                                                              \
    _init(&ctx->inner);                                                       \
    _update(&ctx->inner, blk_inner, sizeof blk_inner);                        \
                                                                              \
    /* And outer. */                                                          \
    _init(&ctx->outer);                                                       \
    _update(&ctx->outer, blk_outer, sizeof blk_outer);                        \
  }                                                                           \
                                                                              \
  static inline void HMAC_UPDATE(_name)(HMAC_CTX(_name) *ctx,                 \
                                        const void *data, size_t ndata)       \
  {                                                                           \
    _update(&ctx->inner, data, ndata);                                        \
  }                                                                           \
                                                                              \
  static inline void HMAC_FINAL(_name)(HMAC_CTX(_name) *ctx,                  \
                                       uint8_t out[_hashsz])                  \
  {                                                                           \
    _final(&ctx->inner, out);                                                 \
    _update(&ctx->outer, out, _hashsz);                                       \
    _final(&ctx->outer, out);                                                 \
  }                                                                           \
                                                                              \
                                                                              \
  /* --- PBKDF2 --- */                                                        \
  static inline void PBKDF2_F(_name)(const HMAC_CTX(_name) *startctx,         \
                                     uint32_t counter,                        \
                                     const uint8_t *salt, size_t nsalt,       \
                                     uint32_t iterations,                     \
                                     uint8_t *out)                            \
  {                                                                           \
    uint8_t countbuf[4];                                                      \
    write32_be(counter, countbuf);                                            \
                                                                              \
    /* Prepare loop-invariant padding block. */                               \
    uint8_t Ublock[_blocksz];                                                 \
    md_pad(Ublock, _blocksz, _hashsz, _blocksz + _hashsz);                    \
                                                                              \
    /* First iteration:                                                       \
     *   U_1 = PRF(P, S || INT_32_BE(i))                                      \
     */                                                                       \
    HMAC_CTX(_name) ctx = *startctx;                                          \
    HMAC_UPDATE(_name)(&ctx, salt, nsalt);                                    \
    HMAC_UPDATE(_name)(&ctx, countbuf, sizeof countbuf);                      \
    HMAC_FINAL(_name)(&ctx, Ublock);                                          \
    _ctx result = ctx.outer;                                                  \
                                                                              \
    /* Subsequent iterations:                                                 \
     *   U_c = PRF(P, U_{c-1})                                                \
     */                                                                       \
    uint32_t i = 0;                                                           \
    for ( i = 1; i < iterations; i++)                                         \
    {                                                                         \
      /* Complete inner hash with previous U */                               \
      _xcpy(&ctx.inner, &startctx->inner);                                    \
      _xform(&ctx.inner, Ublock);                                             \
      _xtract(&ctx.inner, Ublock);                                            \
      /* Complete outer hash with inner output */                             \
      _xcpy(&ctx.outer, &startctx->outer);                                    \
      _xform(&ctx.outer, Ublock);                                             \
      _xtract(&ctx.outer, Ublock);                                            \
      _xxor(&result, &ctx.outer);                                             \
    }                                                                         \
                                                                              \
    /* Reform result into output buffer. */                                   \
    _xtract(&result, out);                                                    \
  }                                                                           \
                                                                              \
  static inline void PBKDF2(_name)(const uint8_t *pw, size_t npw,             \
                     const uint8_t *salt, size_t nsalt,                       \
                     uint32_t iterations,                                     \
                     uint8_t *out, size_t nout)                               \
  {                                                                           \
    assert(iterations);                                                       \
    assert(out && nout);                                                      \
                                                                              \
    /* Starting point for inner loop. */                                      \
    HMAC_CTX(_name) ctx;                                                      \
    HMAC_INIT(_name)(&ctx, pw, npw);                                          \
                                                                              \
    /* How many blocks do we need? */                                         \
    uint32_t blocks_needed = (uint32_t)(nout + _hashsz - 1) / _hashsz;        \
                                                                              \
    OPENMP_PARALLEL_FOR                                                       \
    uint32_t counter = 1;                                                     \
    for (counter = 1; counter <= blocks_needed; counter++)                    \
    {                                                                         \
      uint8_t block[_hashsz];                                                 \
      PBKDF2_F(_name)(&ctx, counter, salt, nsalt, iterations, block);         \
                                                                              \
      size_t offset = (counter - 1) * _hashsz;                                \
      size_t taken = MIN(nout - offset, _hashsz);                             \
      memcpy(out + offset, block, taken);                                     \
    }                                                                         \
  }

#ifdef _MSC_VER
static inline void sha256_extract(SHA256_CTX *restrict ctx, uint8_t *restrict out)
#else
static inline void sha256_extract(SHA256_CTX *ctx, uint8_t *out)
#endif
{
	write32_be(ctx->state[0], out);
	write32_be(ctx->state[1], out + 4);
	write32_be(ctx->state[2], out + 8);
	write32_be(ctx->state[3], out + 12);
	write32_be(ctx->state[4], out + 16);
	write32_be(ctx->state[5], out + 20);
	write32_be(ctx->state[6], out + 24);
	write32_be(ctx->state[7], out + 28);
}

#ifdef _MSC_VER
static inline void sha256_cpy(SHA256_CTX *restrict out, const SHA256_CTX *restrict in)
#else
static inline void sha256_cpy(SHA256_CTX *out, const SHA256_CTX *in)
#endif
{
	out->state[0] = in->state[0];
	out->state[1] = in->state[1];
	out->state[2] = in->state[2];
	out->state[3] = in->state[3];
	out->state[4] = in->state[4];
	out->state[5] = in->state[5];
	out->state[6] = in->state[6];
	out->state[7] = in->state[7];
}

#ifdef _MSC_VER
static inline void sha256_xor(SHA256_CTX *restrict out, const SHA256_CTX *restrict in)
#else
static inline void sha256_xor(SHA256_CTX *out, const SHA256_CTX *in)
#endif
{
	out->state[0] ^= in->state[0];
	out->state[1] ^= in->state[1];
	out->state[2] ^= in->state[2];
	out->state[3] ^= in->state[3];
	out->state[4] ^= in->state[4];
	out->state[5] ^= in->state[5];
	out->state[6] ^= in->state[6];
	out->state[7] ^= in->state[7];
}

DECL_PBKDF2(sha256,
						64,
						32,
						SHA256_CTX,
						_librist_SHA256_Init,
						_librist_SHA256_Update,
						_librist_SHA256_Transform,
						_librist_SHA256_Final,
						sha256_cpy,
						sha256_extract,
						sha256_xor)

void fastpbkdf2_hmac_sha256(const uint8_t *pw, size_t npw,
														const uint8_t *salt, size_t nsalt,
														uint32_t iterations,
														uint8_t *out, size_t nout)
{
	PBKDF2(sha256)(pw, npw, salt, nsalt, iterations, out, nout);
}
