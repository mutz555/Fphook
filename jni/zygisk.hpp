#pragma once

#include <unistd.h>
#include <jni.h>
#include <vector>
#include <functional>
#include <string_view>
#include <pthread.h>

// Zygisk API version
#define ZYGISK_API 4

namespace zygisk {

// Defines how API should be exported
#if defined(ANDROID)
#define ZYGISK_EXPORT extern "C" __attribute__((visibility("default")))
#elif defined(_WIN32)
#define ZYGISK_EXPORT extern "C" __declspec(dllexport)
#else
#error Unknown platform
#endif

// Zygisk module priority
enum : int {
    PRIORITY_DEFAULT = 0,
    PRIORITY_HIGHER = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_MAX = 3,
};

// Zygisk module API callbacks
struct Api {
    // This function is called when the module is loaded into the target process
    // Returns the priority of the module (default, higher, high, max)
    virtual int getApiVersion() = 0;

    // Set callback function to be called when a new process is forked by Zygote
    virtual void setOption(Option opt) = 0;

    // Connect to a companion process
    virtual bool connectCompanion() = 0;

    // Get the file descriptor for a module file
    virtual int getModuleDir() = 0;

    enum Option : int {
        // Register hook for forks
        FORCE_DENYLIST_UNMOUNT = 0,
        DLCLOSE_MODULE_LIBRARY = 1,
    };
};

// Zygisk module implementation interface
struct ModuleBase {
    // Called when the module is loaded into the zygote process
    virtual void onLoad(Api *api, JNIEnv *env) = 0;

    // Called when a new process is being forked from zygote
    virtual void preAppSpecialize(Api *api, JNIEnv *env,
                                 jclass clazz,
                                 jint *uid,
                                 jint *gid,
                                 jintArray *gids,
                                 jint *runtimeFlags,
                                 jobjectArray *rlimits,
                                 jint *mountExternal,
                                 jstring *seInfo,
                                 jstring *niceName,
                                 jintArray *fdsToClose,
                                 jintArray *fdsToIgnore,
                                 jboolean *is_child_zygote,
                                 jstring *instructionSet,
                                 jstring *appDataDir,
                                 jboolean *isTopApp,
                                 jobjectArray *pkgDataInfoList,
                                 jobjectArray *whitelistedDataInfoList,
                                 jboolean *bindMountAppDataDirs,
                                 jboolean *bindMountAppStorageDirs) {}

    // Called in app process after fork
    virtual void postAppSpecialize(Api *api, JNIEnv *env, jclass clazz) {}

    // Called in system_server process
    virtual void preServerSpecialize(Api *api, JNIEnv *env, jclass clazz) {}

    // Called in system_server after fork
    virtual void postServerSpecialize(Api *api, JNIEnv *env, jclass clazz) {}
};

// Companion process callback
struct CompanionBase {
    // Called when the companion process is started
    virtual void onCompanionStart(JNIEnv *env) = 0;
};

// Register functions for Zygisk API
inline int entry(void *handle, void *args) {
    auto _entry = reinterpret_cast<int(*)(void*, void*)>(args);
    return _entry != nullptr ? _entry(handle, nullptr) : -1;
}

inline void registerModule(ModuleBase *module) {
    static ModuleBase *_module = module;
    ZYGISK_EXPORT int zygisk_module_entry(void *handle, void *args);
}

} // namespace zygisk

// Define the module entry point
ZYGISK_EXPORT int zygisk_module_entry(void *handle, void *args) {
    return zygisk::entry(handle, args);
}
