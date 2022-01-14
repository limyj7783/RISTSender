 
# Copyright (C) 2009 The Android Open Source Project
#  
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#   
#      http://www.apache.org/licenses/LICENSE-2.0
#   
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
     
        
LOCAL_PATH := $(call my-dir)
APP_CPPFLAGS := -fexceptions


include $(CLEAR_VARS)
#################################################
LOCAL_MODULE				:= ristprotocol

LOCAL_CFLAGS 				+= -DUSE_NOUGAT

LOCAL_C_INCLUDES 	:=  $(LOCAL_PATH)/external/rist/build/include/librist \
                        $(LOCAL_PATH)/external/rist/build/include \
                        $(LOCAL_PATH)/external/rist/contrib \
                        $(LOCAL_PATH)/external/rist/contrib/contrib_cJSON \
                        $(LOCAL_PATH)/external/rist/contrib/mbedtls/include \
                        $(LOCAL_PATH)/external/rist/contrib/mbedtls/include/mbedtls \
                        $(LOCAL_PATH)/external/rist/contrib/mbedtls/include/psa \
                        $(LOCAL_PATH)/external/rist/contrib/mbedtls/library \
                        $(LOCAL_PATH)/external/rist/include \
                        $(LOCAL_PATH)/external/rist/include/common \
                        $(LOCAL_PATH)/external/rist/include/librist \
                        $(LOCAL_PATH)/external/rist/src \
                        $(LOCAL_PATH)/external/rist/src/crypto \
                        $(LOCAL_PATH)

LOCAL_SRC_FILES		+= external/rist/build/include/librist/librist_config.h \
                       external/rist/build/include/librist/version.h \
                       external/rist/build/include/version.h \
                       external/rist/contrib/contrib_cJSON/cjson/cJSON.c \
                       external/rist/contrib/contrib_cJSON/cjson/cJSON.h \
                       external/rist/contrib/mbedtls/include/mbedtls/aes.h \
                       external/rist/contrib/mbedtls/include/mbedtls/aesni.h \
                       external/rist/contrib/mbedtls/include/mbedtls/arc4.h \
                       external/rist/contrib/mbedtls/include/mbedtls/aria.h \
                       external/rist/contrib/mbedtls/include/mbedtls/asn1.h \
                       external/rist/contrib/mbedtls/include/mbedtls/asn1write.h \
                       external/rist/contrib/mbedtls/include/mbedtls/base64.h \
                       external/rist/contrib/mbedtls/include/mbedtls/bignum.h \
                       external/rist/contrib/mbedtls/include/mbedtls/blowfish.h \
                       external/rist/contrib/mbedtls/include/mbedtls/bn_mul.h \
                       external/rist/contrib/mbedtls/include/mbedtls/camellia.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ccm.h \
                       external/rist/contrib/mbedtls/include/mbedtls/certs.h \
                       external/rist/contrib/mbedtls/include/mbedtls/chacha20.h \
                       external/rist/contrib/mbedtls/include/mbedtls/chachapoly.h \
                       external/rist/contrib/mbedtls/include/mbedtls/check_config.h \
                       external/rist/contrib/mbedtls/include/mbedtls/cipher.h \
                       external/rist/contrib/mbedtls/include/mbedtls/cipher_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/cmac.h \
                       external/rist/contrib/mbedtls/include/mbedtls/compat-1.3.h \
                       external/rist/contrib/mbedtls/include/mbedtls/config.h \
                       external/rist/contrib/mbedtls/include/mbedtls/config_psa.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ctr_drbg.h \
                       external/rist/contrib/mbedtls/include/mbedtls/debug.h \
                       external/rist/contrib/mbedtls/include/mbedtls/des.h \
                       external/rist/contrib/mbedtls/include/mbedtls/dhm.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ecdh.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ecdsa.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ecjpake.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ecp.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ecp_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/entropy.h \
                       external/rist/contrib/mbedtls/include/mbedtls/entropy_poll.h \
                       external/rist/contrib/mbedtls/include/mbedtls/error.h \
                       external/rist/contrib/mbedtls/include/mbedtls/gcm.h \
                       external/rist/contrib/mbedtls/include/mbedtls/havege.h \
                       external/rist/contrib/mbedtls/include/mbedtls/hkdf.h \
                       external/rist/contrib/mbedtls/include/mbedtls/hmac_drbg.h \
                       external/rist/contrib/mbedtls/include/mbedtls/md.h \
                       external/rist/contrib/mbedtls/include/mbedtls/md2.h \
                       external/rist/contrib/mbedtls/include/mbedtls/md4.h \
                       external/rist/contrib/mbedtls/include/mbedtls/md5.h \
                       external/rist/contrib/mbedtls/include/mbedtls/md_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/memory_buffer_alloc.h \
                       external/rist/contrib/mbedtls/include/mbedtls/net.h \
                       external/rist/contrib/mbedtls/include/mbedtls/net_sockets.h \
                       external/rist/contrib/mbedtls/include/mbedtls/nist_kw.h \
                       external/rist/contrib/mbedtls/include/mbedtls/oid.h \
                       external/rist/contrib/mbedtls/include/mbedtls/padlock.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pem.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pk.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pkcs11.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pkcs12.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pkcs5.h \
                       external/rist/contrib/mbedtls/include/mbedtls/pk_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/platform.h \
                       external/rist/contrib/mbedtls/include/mbedtls/platform_time.h \
                       external/rist/contrib/mbedtls/include/mbedtls/platform_util.h \
                       external/rist/contrib/mbedtls/include/mbedtls/poly1305.h \
                       external/rist/contrib/mbedtls/include/mbedtls/psa_util.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ripemd160.h \
                       external/rist/contrib/mbedtls/include/mbedtls/rsa.h \
                       external/rist/contrib/mbedtls/include/mbedtls/rsa_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/sha1.h \
                       external/rist/contrib/mbedtls/include/mbedtls/sha256.h \
                       external/rist/contrib/mbedtls/include/mbedtls/sha512.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl_cache.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl_ciphersuites.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl_cookie.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl_internal.h \
                       external/rist/contrib/mbedtls/include/mbedtls/ssl_ticket.h \
                       external/rist/contrib/mbedtls/include/mbedtls/threading.h \
                       external/rist/contrib/mbedtls/include/mbedtls/timing.h \
                       external/rist/contrib/mbedtls/include/mbedtls/version.h \
                       external/rist/contrib/mbedtls/include/mbedtls/x509.h \
                       external/rist/contrib/mbedtls/include/mbedtls/x509_crl.h \
                       external/rist/contrib/mbedtls/include/mbedtls/x509_crt.h \
                       external/rist/contrib/mbedtls/include/mbedtls/x509_csr.h \
                       external/rist/contrib/mbedtls/include/mbedtls/xtea.h \
                       external/rist/contrib/mbedtls/include/psa/crypto.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_compat.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_config.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_driver_common.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_extra.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_platform.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_se_driver.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_sizes.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_struct.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_types.h \
                       external/rist/contrib/mbedtls/include/psa/crypto_values.h \
                       external/rist/contrib/mbedtls/library/aes.c \
                       external/rist/contrib/mbedtls/library/aesni.c \
                       external/rist/contrib/mbedtls/library/arc4.c \
                       external/rist/contrib/mbedtls/library/aria.c \
                       external/rist/contrib/mbedtls/library/asn1parse.c \
                       external/rist/contrib/mbedtls/library/asn1write.c \
                       external/rist/contrib/mbedtls/library/base64.c \
                       external/rist/contrib/mbedtls/library/bignum.c \
                       external/rist/contrib/mbedtls/library/blowfish.c \
                       external/rist/contrib/mbedtls/library/camellia.c \
                       external/rist/contrib/mbedtls/library/ccm.c \
                       external/rist/contrib/mbedtls/library/certs.c \
                       external/rist/contrib/mbedtls/library/chacha20.c \
                       external/rist/contrib/mbedtls/library/chachapoly.c \
                       external/rist/contrib/mbedtls/library/check_crypto_config.h \
                       external/rist/contrib/mbedtls/library/cipher.c \
                       external/rist/contrib/mbedtls/library/cipher_wrap.c \
                       external/rist/contrib/mbedtls/library/cmac.c \
                       external/rist/contrib/mbedtls/library/common.h \
                       external/rist/contrib/mbedtls/library/ctr_drbg.c \
                       external/rist/contrib/mbedtls/library/debug.c \
                       external/rist/contrib/mbedtls/library/des.c \
                       external/rist/contrib/mbedtls/library/dhm.c \
                       external/rist/contrib/mbedtls/library/ecdh.c \
                       external/rist/contrib/mbedtls/library/ecdsa.c \
                       external/rist/contrib/mbedtls/library/ecjpake.c \
                       external/rist/contrib/mbedtls/library/ecp.c \
                       external/rist/contrib/mbedtls/library/ecp_curves.c \
                       external/rist/contrib/mbedtls/library/entropy.c \
                       external/rist/contrib/mbedtls/library/entropy_poll.c \
                       external/rist/contrib/mbedtls/library/error.c \
                       external/rist/contrib/mbedtls/library/gcm.c \
                       external/rist/contrib/mbedtls/library/havege.c \
                       external/rist/contrib/mbedtls/library/hkdf.c \
                       external/rist/contrib/mbedtls/library/hmac_drbg.c \
                       external/rist/contrib/mbedtls/library/md.c \
                       external/rist/contrib/mbedtls/library/md2.c \
                       external/rist/contrib/mbedtls/library/md4.c \
                       external/rist/contrib/mbedtls/library/md5.c \
                       external/rist/contrib/mbedtls/library/memory_buffer_alloc.c \
                       external/rist/contrib/mbedtls/library/net_sockets.c \
                       external/rist/contrib/mbedtls/library/nist_kw.c \
                       external/rist/contrib/mbedtls/library/oid.c \
                       external/rist/contrib/mbedtls/library/padlock.c \
                       external/rist/contrib/mbedtls/library/pem.c \
                       external/rist/contrib/mbedtls/library/pk.c \
                       external/rist/contrib/mbedtls/library/pkcs11.c \
                       external/rist/contrib/mbedtls/library/pkcs12.c \
                       external/rist/contrib/mbedtls/library/pkcs5.c \
                       external/rist/contrib/mbedtls/library/pkparse.c \
                       external/rist/contrib/mbedtls/library/pkwrite.c \
                       external/rist/contrib/mbedtls/library/pk_wrap.c \
                       external/rist/contrib/mbedtls/library/platform.c \
                       external/rist/contrib/mbedtls/library/platform_util.c \
                       external/rist/contrib/mbedtls/library/poly1305.c \
                       external/rist/contrib/mbedtls/library/psa_crypto.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_client.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_core.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_driver_wrappers.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_driver_wrappers.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_ecp.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_ecp.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_invasive.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_its.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_random_impl.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_rsa.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_rsa.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_se.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_se.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_service_integration.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_slot_management.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_slot_management.h \
                       external/rist/contrib/mbedtls/library/psa_crypto_storage.c \
                       external/rist/contrib/mbedtls/library/psa_crypto_storage.h \
                       external/rist/contrib/mbedtls/library/psa_its_file.c \
                       external/rist/contrib/mbedtls/library/ripemd160.c \
                       external/rist/contrib/mbedtls/library/rsa.c \
                       external/rist/contrib/mbedtls/library/rsa_internal.c \
                       external/rist/contrib/mbedtls/library/sha1.c \
                       external/rist/contrib/mbedtls/library/sha256.c \
                       external/rist/contrib/mbedtls/library/sha512.c \
                       external/rist/contrib/mbedtls/library/ssl_cache.c \
                       external/rist/contrib/mbedtls/library/ssl_ciphersuites.c \
                       external/rist/contrib/mbedtls/library/ssl_cli.c \
                       external/rist/contrib/mbedtls/library/ssl_cookie.c \
                       external/rist/contrib/mbedtls/library/ssl_invasive.h \
                       external/rist/contrib/mbedtls/library/ssl_msg.c \
                       external/rist/contrib/mbedtls/library/ssl_srv.c \
                       external/rist/contrib/mbedtls/library/ssl_ticket.c \
                       external/rist/contrib/mbedtls/library/ssl_tls.c \
                       external/rist/contrib/mbedtls/library/ssl_tls13_keys.c \
                       external/rist/contrib/mbedtls/library/ssl_tls13_keys.h \
                       external/rist/contrib/mbedtls/library/threading.c \
                       external/rist/contrib/mbedtls/library/timing.c \
                       external/rist/contrib/mbedtls/library/version.c \
                       external/rist/contrib/mbedtls/library/version_features.c \
                       external/rist/contrib/mbedtls/library/x509.c \
                       external/rist/contrib/mbedtls/library/x509write_crt.c \
                       external/rist/contrib/mbedtls/library/x509write_csr.c \
                       external/rist/contrib/mbedtls/library/x509_create.c \
                       external/rist/contrib/mbedtls/library/x509_crl.c \
                       external/rist/contrib/mbedtls/library/x509_crt.c \
                       external/rist/contrib/mbedtls/library/x509_csr.c \
                       external/rist/contrib/mbedtls/library/xtea.c \
                       external/rist/contrib/aes.c \
                       external/rist/contrib/aes.h \
                       external/rist/contrib/endian-shim.h \
                       external/rist/contrib/fastpbkdf2.c \
                       external/rist/contrib/fastpbkdf2.h \
                       external/rist/contrib/getopt-shim.c \
                       external/rist/contrib/getopt-shim.h \
                       external/rist/contrib/linux-crypto.c \
                       external/rist/contrib/linux-crypto.h \
                       external/rist/contrib/poll_win.c \
                       external/rist/contrib/pthread-shim.c \
                       external/rist/contrib/pthread-shim.h \
                       external/rist/contrib/sha256.c \
                       external/rist/contrib/sha256.h \
                       external/rist/contrib/socket-shim.h \
                       external/rist/contrib/srp.c \
                       external/rist/contrib/srp.h \
                       external/rist/contrib/stdio-shim.c \
                       external/rist/contrib/stdio-shim.h \
                       external/rist/contrib/time-shim.c \
                       external/rist/contrib/time-shim.h \
                       external/rist/include/common/attributes.h \
                       external/rist/include/librist/common.h \
                       external/rist/include/librist/headers.h \
                       external/rist/include/librist/librist.h \
                       external/rist/include/librist/librist_srp.h \
                       external/rist/include/librist/logging.h \
                       external/rist/include/librist/oob.h \
                       external/rist/include/librist/opt.h \
                       external/rist/include/librist/peer.h \
                       external/rist/include/librist/receiver.h \
                       external/rist/include/librist/sender.h \
                       external/rist/include/librist/stats.h \
                       external/rist/include/librist/udpsocket.h \
                       external/rist/include/librist/urlparam.h \
                       external/rist/src/crypto/crypto-private.h \
                       external/rist/src/crypto/crypto.c \
                       external/rist/src/crypto/psk.c \
                       external/rist/src/crypto/psk.h \
                       external/rist/src/eap.c \
                       external/rist/src/eap.h \
                       external/rist/src/flow.c \
                       external/rist/src/libevsocket.c \
                       external/rist/src/libevsocket.h \
                       external/rist/src/log-private.h \
                       external/rist/src/logging.c \
                       external/rist/src/mpegts.c \
                       external/rist/src/mpegts.h \
                       external/rist/src/rist-common.c \
                       external/rist/src/rist-private.h \
                       external/rist/src/rist-thread.c \
                       external/rist/src/rist-thread.h \
                       external/rist/src/rist.c \
                       external/rist/src/rist_ref.c \
                       external/rist/src/rist_ref.h \
                       external/rist/src/stats.c \
                       external/rist/src/udp-private.h \
                       external/rist/src/udp.c \
                       external/rist/src/udpsocket.c \
                       RISTSenderInterface.cpp

LOCAL_CFLAGS 		+=  -Wno-multichar -fpermissive
LOCAL_LDLIBS  		+=	-landroid -llog -lEGL -lGLESv2 -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)