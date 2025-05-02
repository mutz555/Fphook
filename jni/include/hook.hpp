#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <dobby.h>
#include *logging.hpp"
#include "lsplant.hpp"

class JavaMethodHook {
public:
    JavaMethodHook(const std::string& className,
                   const std::string& methodName,
                   const std::string& methodSignature,
                   void* replacement,
                   void** original)
        : className(className), methodName(methodName),
          methodSignature(methodSignature),
          replacement(replacement), original(original) {}

    bool apply(JNIEnv* env);

private:
    std::string className;
    std::string methodName;
    std::string methodSignature;
    void* replacement;
    void** original;
};

class HookManager {
public:
    static HookManager& getInstance() {
        static HookManager instance;
        return instance;
    }

    void addJavaHook(const std::shared_ptr<JavaMethodHook>& hook);
    void applyJavaHooks(JNIEnv* env);

private:
    std::vector<std::shared_ptr<JavaMethodHook>> javaHooks;
};