#include "log.h"

#include <android/log.h>
#include <stdio.h>

static jmethodID logID = nullptr;


bool log_OnLoad(JNIEnv *env)
{
    jclass activity = env->FindClass("online/adamek/sten/surfacing/MainActivity");
    if (activity == nullptr)
        return false;

    logID = env->GetMethodID(activity, "log", "(Ljava/lang/String;)V");
    if (logID == nullptr)
        return false;

    return true;
}


void log_print(JNIEnv *env,
               jobject activity,
               char const* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    __android_log_write(ANDROID_LOG_INFO, "Surfacing", buf);
    env->CallVoidMethod(activity, logID, env->NewStringUTF(buf));
}
