#pragma once

#include <jni.h>

namespace zygisk {

class Api {
public:
    virtual void preAppSpecialize(Api *api, JNIEnv *env, jclass clazz) { }
    virtual void postAppSpecialize(Api *api, JNIEnv *env, jclass clazz) { }
    virtual void preServerSpecialize(Api *api, JNIEnv *env, jclass clazz) { }
    virtual void postServerSpecialize(Api *api, JNIEnv *env, jclass clazz) { }
};

class ModuleBase {
public:
    virtual ~ModuleBase() = default;

    virtual void onLoad(Api* api, JNIEnv* env) { }
    virtual void preAppSpecialize(Api* api, JNIEnv* env,
                                  jclass clazz, jint* uid, jint* gid,
                                  jintArray* gids, jint* runtimeFlags,
                                  jobjectArray* rlimits, jint* mountExternal,
                                  jstring* seInfo, jstring* niceName,
                                  jintArray* fdsToClose, jintArray* fdsToIgnore,
                                  jboolean* is_child_zygote,
                                  jstring* instructionSet, jstring* appDataDir,
                                  jboolean* isTopApp,
                                  jobjectArray* pkgDataInfoList,
                                  jobjectArray* whitelistedDataInfoList,
                                  jboolean* bindMountAppDataDirs,
                                  jboolean* bindMountAppStorageDirs) { }

    virtual void postAppSpecialize(Api* api, JNIEnv* env, jclass clazz) { }
    virtual void preServerSpecialize(Api* api, JNIEnv* env, jclass clazz) { }
    virtual void postServerSpecialize(Api* api, JNIEnv* env, jclass clazz) { }
};

// Simulasi entrypoint dan registerModule
inline void registerModule(ModuleBase* module) {
    // Dummy fungsi, kosongkan
}

inline int entry(void* handle, void* args) {
    return 0; // Dummy implementasi
}

} // namespace zygisk

// Export macro
#ifndef ZYGISK_EXPORT
#define ZYGISK_EXPORT extern "C" __attribute__((visibility("default")))
#endif