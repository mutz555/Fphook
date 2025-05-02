#pragma once

#include <jni.h>
#include <android/log.h>
#include <string>
#include <filesystem>
#include <cstdio>

#define LOG_TAG "FpBypass"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class Logger {
public:
    static void setLogFile(const std::string& path) {
        logFile = path;
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
        if (logFile.empty()) return;

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
                    message.c_str());
            fclose(file);
        }
    }

    static std::string getCurrentTimeString() {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
        return std::string(buf);
    }

    static inline std::string logFile;
};