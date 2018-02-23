#include <jni.h>

#include "log.h"
#include "MainActivity.h"


extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(
        JavaVM* vm,
        void* /*reserved*/)
{
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK)
        return 0;

    if (!log_OnLoad(env))
        return 0;

    if (!MainActivity_OnLoad(env))
        return 0;

    return JNI_VERSION_1_4;
}
