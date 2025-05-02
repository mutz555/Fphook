#pragma once
#include <android/log.h>

#define TAG "FPBypass"

class Logger {
public:
    static void debug(const char* msg) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", msg);
    }

    static void warn(const char* msg) {
        __android_log_print(ANDROID_LOG_WARN, TAG, "%s", msg);
    }

    // Versi overload untuk string C++
    static void debug(const std::string& msg) {
        debug(msg.c_str());
    }

    static void warn(const std::string& msg) {
        warn(msg.c_str());
    }
};