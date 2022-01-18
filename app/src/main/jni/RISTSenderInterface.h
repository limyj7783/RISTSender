#ifndef _Included_com_example_ristsender_RISTSenderInterface
#define _Included_com_example_ristsender_RISTSenderInterface
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

#include "RISTDefine.h"
//#include "RISTFunction.h"
#include <android/log.h>

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_getTestValue(JNIEnv *, jobject);
JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_SendStart(JNIEnv *, jobject, jstring, jint);
JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_SendStop(JNIEnv *, jobject);

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStart(JNIEnv *, jobject);

static struct rist_peer* setup_rist_peer(struct rist_sender_args *setup);
static void input_udp_recv(struct evsocket_ctx *evctx, int fd, short revents, void *arg);
static void input_udp_sockerr(struct evsocket_ctx *evctx, int fd, short revents, void *arg);
static PTHREAD_START_FUNC(input_loop, arg);
static int cb_stats(void *arg, const struct rist_stats *stats_container);
static int cb_auth_connect(void *arg, const char* connecting_ip, uint16_t connecting_port, const char* local_ip, uint16_t local_port, struct rist_peer *peer);
static int cb_auth_disconnect(void *arg, struct rist_peer *peer);
static void connection_status_callback(void *arg, struct rist_peer *peer, enum rist_connection_status peer_connection_status);
static int cb_recv_oob(void *arg, const struct rist_oob_block *oob_block);
#ifdef __cplusplus
}
#endif
#endif