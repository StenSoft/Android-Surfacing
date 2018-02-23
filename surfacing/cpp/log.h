#pragma once

#include <jni.h>

/**
 * Called from JNI_OnLoad.
 */
bool log_OnLoad(JNIEnv *env);

void log_print(JNIEnv *env,
               jobject activity,
               char const* fmt, ...);

/**
 * Log a message.
 *
 * @param fmt... printf-style formatted message.
 */
#define log(fmt...) log_print(env, activity, fmt)
