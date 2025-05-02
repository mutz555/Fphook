#pragma once

#include <functional>
#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "dobby.h"
#include "lsplant.hpp"
#include "logging.hpp"
#include "config.hpp"

class JavaMethodHook {
public:
    JavaMethodHook(const std::string& className,
                   const std::string& methodName,
                   const std::string& methodSig,
                   std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> replacement,
                   std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> original);

    bool install(JNIEnv* env);

private:
    std::string cls;
    std::string mth;
    std::string sig;
    std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> repl;
    std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> orig;
};

class HookManager {
public:
    static HookManager& getInstance();
    void addJavaHook(std::unique_ptr<JavaMethodHook> hook);
    bool initialize(JNIEnv* env);

private:
    HookManager() = default;
    std::vector<std::unique_ptr<JavaMethodHook>> javaHooks;
};