#ifndef _Included_com_example_ristsender_RISTSenderInterface
#define _Included_com_example_ristsender_RISTSenderInterface
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include "RISTDefine.h"

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStart(JNIEnv *, jobject);
JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStop(JNIEnv *, jobject);

static int cb_auth_connect(void *arg, const char* connecting_ip, uint16_t connecting_port, const char* local_ip, uint16_t local_port, struct rist_peer *peer);
static int cb_auth_disconnect(void *arg, struct rist_peer *peer);
static void connection_status_callback(void *arg, struct rist_peer *peer, enum rist_connection_status peer_connection_status);
static int cb_recv_oob(void *arg, const struct rist_oob_block *oob_block);
static int cb_stats(void *arg, const struct rist_stats *stats_container);
static int cb_recv(void *arg, struct rist_data_block *b);
static uint32_t risttools_convertNTPtoRTP(uint64_t i_ntp);
static inline void risttools_rtp_set_hdr(uint8_t *p_rtp, uint8_t i_type, uint16_t i_seqnum, uint32_t i_timestamp, uint32_t i_ssrc);
#ifdef __cplusplus
}
#endif
#endif
