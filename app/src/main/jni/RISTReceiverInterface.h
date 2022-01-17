#ifndef _Included_com_example_ristsender_RISTSenderInterface
#define _Included_com_example_ristsender_RISTSenderInterface
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include "RISTDefine.h"
#include "RISTFunction.h"

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStart(JNIEnv *, jobject);
JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStop(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
