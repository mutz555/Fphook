#pragma once

#include <android/log.h>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

#define TAG "FPBypassModule"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// For debug mode, enable verbose logging
class Logger {
private:
    static bool debugEnabled;
    static std::string logFile;
    
    static std::string getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
public:
    static void setDebugEnabled(bool enabled) {
        debugEnabled = enabled;
    }
    
    static bool isDebugEnabled() {
        return debugEnabled;
    }
    
    static void setLogFile(const std::string& path) {
        logFile = path;
    }
    
    static void debug(const std::string& message) {
        if (debugEnabled) {
            LOGD("%s", message.c_str());
            writeToFile("DEBUG", message);
        }
    }
    
    static void info(const std::string& message) {
        LOGI("%s", message.c_str());
        writeToFile("INFO", message);
    }
    
    static void warn(const std::string& message) {
        LOGW("%s", message.c_str());
        writeToFile("WARN", message);
    }
    
    static void error(const std::string& message) {
        LOGE("%s", message.c_str());
        writeToFile("ERROR", message);
    }
    
private:
    static void writeToFile(const std::string& level, const std::string& message) {
        if (logFile.empty()) return;
        
        try {
            // Ensure directory exists
            std::filesystem::path path(logFile);
            if (!path.parent_path().empty()) {
                std::filesystem::create_directories(path.parent_path());
            }
            
            // Append to log file
            FILE* file = fopen(logFile.c_str(), "a");
            if (file) {
                fprintf(file, "[%s] [%s] %s\n", 
                        getCurrentTimeString().c_str(), 
                        level.c_str(), 
                        message.c_str());
                fclose(file);
            }
        } catch (...) {
            // Ignore file write errors
        }
    }
};

// Initialize static members
bool Logger::debugEnabled = false;
std::string Logger::logFile = "";
