#include "module.hpp"
#include <string>
#include <unistd.h>

namespace fpbypass {

FingerprintBypassModule::FingerprintBypassModule() {
    Logger::info("FingerprintBypassModule constructed");
}

FingerprintBypassModule::~FingerprintBypassModule() {
    Logger::info("FingerprintBypassModule destroyed");
}

void FingerprintBypassModule::onLoad(zygisk::Api* api, JNIEnv* env) {
    this->api = api;
    this->env = env;
    
    // Set module priority
    api->setOption(zygisk::Api::Option::DLCLOSE_MODULE_LIBRARY);
    
    // Initialize configuration
    std::string configPath = "/data/adb/modules/FingerprintBypass/config.json";
    if (!Configuration::getInstance().initialize(configPath)) {
        Logger::error("Failed to initialize configuration, using defaults");
    }
    
    Logger::info("FingerprintBypassModule loaded");
}

void FingerprintBypassModule::preAppSpecialize(zygisk::Api* api, JNIEnv* env,
                                            jclass,
                                            jint*,
                                            jint*,
                                            jintArray*,
                                            jint*,
                                            jobjectArray*,
                                            jint*,
                                            jstring* seInfo,
                                            jstring* niceName,
                                            jintArray*,
                                            jintArray*,
                                            jboolean*,
                                            jstring*,
                                            jstring* appDataDir,
                                            jboolean*,
                                            jobjectArray*,
                                            jobjectArray*,
                                            jboolean*,
                                            jboolean*) {
    if (appDataDir == nullptr) {
        return;
    }
    
    // Get package name from app data dir
    currentPackageName = getPackageNameFromAppDataDir(env, *appDataDir);
    
    // Check if we should hook this process
    if (!shouldHookCurrentProcess(currentPackageName)) {
        api->setOption(zygisk::Api::Option::DLCLOSE_MODULE_LIBRARY);
        return;
    }
    
    Logger::info("Pre-specializing for " + currentPackageName);
    isSystemServer = false;
}

void FingerprintBypassModule::postAppSpecialize(zygisk::Api* api, JNIEnv* env, jclass) {
    if (currentPackageName.empty()) {
        return;
    }
    
    Logger::info("Post-specializing for " + currentPackageName);
    
    // Initialize hook manager
    if (!HookManager::getInstance().initialize(env)) {
        Logger::error("Failed to initialize hook manager");
        return;
    }
    
    // Apply hooks
    if (!HookManager::getInstance().applyHooks()) {
        Logger::error("Failed to apply hooks");
    }
}

void FingerprintBypassModule::preServerSpecialize(zygisk::Api* api, JNIEnv* env, jclass) {
    isSystemServer = true;
    currentPackageName = "android.server";
    Logger::info("Pre-specializing for system_server");
}

void FingerprintBypassModule::postServerSpecialize(zygisk::Api* api, JNIEnv* env, jclass) {
    Logger::info("Post-specializing for system_server");
    
    // Initialize hook manager
    if (!HookManager::getInstance().initialize(env)) {
        Logger::error("Failed to initialize hook manager");
        return;
    }
    
    // Apply hooks
    if (!HookManager::getInstance().applyHooks()) {
        Logger::error("Failed to apply hooks");
    }
}

std::string FingerprintBypassModule::getPackageNameFromAppDataDir(JNIEnv* env, jstring appDataDir) {
    if (appDataDir == nullptr) {
        return "";
    }
    
    const char* dir = env->GetStringUTFChars(appDataDir, nullptr);
    if (dir == nullptr) {
        return "";
    }
    
    std::string dirStr(dir);
    env->ReleaseStringUTFChars(appDataDir, dir);
    
    // Extract package name from directory path
    // Format usually like /data/user/0/com.example.app
    size_t lastSlash = dirStr.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return dirStr.substr(lastSlash + 1);
    }
    
    return dirStr;
}

bool FingerprintBypassModule::shouldHookCurrentProcess(const std::string& packageName) {
    return HookManager::getInstance().shouldHookPackage(packageName);
}

// Register the module
static FingerprintBypassModule module;

} // namespace fpbypass

// Register module
ZYGISK_EXPORT int zygisk_module_entry(void* handle, void* args) {
    return zygisk::entry(handle, args);
}
