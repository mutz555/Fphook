#pragma once

#include <jni.h>
#include <android/log.h>
#include <string>
#include <filesystem>
#include <cstdio>
#include <ctime>
#include <error.h>

#define LOG_TAG "FpBypass"
#define LOGI(...) if (Logger::isDebugEnabled()) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) if (Logger::isDebugEnabled()) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class Logger {
public:
    static void setLogFile(const std::string& path) {
        logFile = path;
    }

    static void setDebugEnabled(bool enabled) {
        debugEnabled = enabled;
    }

    static bool isDebugEnabled() {
        return debugEnabled;
    }

    static void info(const std::string& message) {
        LOGI("%s", message.c_str());
        writeToFile("INFO", message);
    }

    static void error(const std::string& message) {
        LOGE("%s", message.c_str());
        writeToFile("ERROR", message);
    }

private:
    static void writeToFile(const std::string& level, const std::string& message) {
        if (!debugEnabled || logFile.empty()) return;

        std::filesystem::path path(logFile);
        if (!path.parent_path().empty()) {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);
        }

        FILE* file = fopen(logFile.c_str(), "a");
        if (file) {
            fprintf(file, "[%s] [%s] %s\n",
                    getCurrentTimeString().c_str(),
                    level.c_str(),
                    message