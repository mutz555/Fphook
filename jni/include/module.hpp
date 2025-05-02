#pragma once

#include "zygisk.hpp"
#include "logging.hpp"
#include "hook.hpp"
#include "config.hpp"

namespace fpbypass {

// Main module class implementing the Zygisk ModuleBase interface
class FingerprintBypassModule : public zygisk::ModuleBase {
public:
    FingerprintBypassModule();
    ~FingerprintBypassModule();
    
    // Called when the module is loaded into the zygote process
    void onLoad(zygisk::Api* api, JNIEnv* env) override;
    
    // Called when a new process is being forked from zygote
    void preAppSpecialize(zygisk::Api* api, JNIEnv* env,
                         jclass clazz,
                         jint* uid,
                         jint* gid,
                         jintArray* gids,
                         jint* runtimeFlags,
                         jobjectArray* rlimits,
                         jint* mountExternal,
                         jstring* seInfo,
                         jstring* niceName,
                         jintArray* fdsToClose,
                         jintArray* fdsToIgnore,
                         jboolean* is_child_zygote,
                         jstring* instructionSet,
                         jstring* appDataDir,
                         jboolean* isTopApp,
                         jobjectArray* pkgDataInfoList,
                         jobjectArray* whitelistedDataInfoList,
                         jboolean* bindMountAppDataDirs,
                         jboolean* bindMountAppStorageDirs) override;
    
    // Called in app process after fork
    void postAppSpecialize(zygisk::Api* api, JNIEnv* env, jclass clazz) override;
    
    // Called in system_server process before fork
    void preServerSpecialize(zygisk::Api* api, JNIEnv* env, jclass clazz) override;
    
    // Called in system_server after fork
    void postServerSpecialize(zygisk::Api* api, JNIEnv* env, jclass clazz) override;
    
private:
    // Get package name from app data dir
    std::string getPackageNameFromAppDataDir(JNIEnv* env, jstring appDataDir);
    
    // Check if we should hook the current process
    bool shouldHookCurrentProcess(const std::string& packageName);
    
    zygisk::Api* api = nullptr;
    JNIEnv* env = nullptr;
    std::string currentPackageName;
    bool isSystemServer = false;
};

} // namespace fpbypass
