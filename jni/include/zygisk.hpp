#pragma once

#include <jni.h>
#include <string>

namespace zygisk {

class Api {
public:
    virtual void preAppSpecialize(Api *api, JNIEnv *env, jclass clazz) {
        (void)api; (void)env; (void)clazz;
    }

    virtual void postAppSpecialize(Api *api, JNIEnv *env, jclass clazz) {
        (void)api; (void)env; (void)clazz;
    }

    virtual void preServerSpecialize(Api *api, JNIEnv *env, jclass clazz) {
        (void)api; (void)env; (void)clazz;
    }

    virtual void postServerSpecialize(Api *api, JNIEnv *env, jclass clazz) {
        (void)api; (void)env; (void)clazz;
    }
};

} // namespace zygisk

#ifndef ZYGISK_EXPORT
#define ZYGISK_EXPORT extern "C" __attribute__((visibility("default")))
#endif

ZYGISK_EXPORT int zygisk_module_entry(void *handle, void *args);