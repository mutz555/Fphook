#include "hook.hpp"
#include <string>
#include <vector>
#include <dobby.h>
#include <thread>
#include <chrono>

JavaMethodHook::JavaMethodHook(const std::string& className,
                               const std::string& methodName,
                               const std::string& methodSig,
                               std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> replacement,
                               std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> original)
  : cls(className), mth(methodName), sig(methodSig), repl(replacement), orig(original) {}

bool JavaMethodHook::install(JNIEnv* env) {
    jclass target = env->FindClass(cls.c_str());
    if (!target) {
        Logger::error("Class not found: " + cls);
        return false;
    }
    jmethodID mid = env->GetMethodID(target, mth.c_str(), sig.c_str());
    if (!mid) {
        Logger::error("MethodID not found: " + mth + " " + sig);
        return false;
    }

    if (lsplant::InitializeInternal(env) != LSPLANT_SUCCESS) {
        Logger::error("lsplant initialization failed");
        return false;
    }

    if (DobbyHook((void*)mid, (void*)repl.target_type().name(), (void**)&orig) != RS_SUCCESS) {
        Logger::error("Failed to hook " + cls + "." + mth);
        return false;
    }

    Logger::info("Hooked " + cls + "." + mth);
    return true;
}

HookManager& HookManager::getInstance() {
    static HookManager instance;
    return instance;
}

void HookManager::addJavaHook(std::unique_ptr<JavaMethodHook> hook) {
    javaHooks.emplace_back(std::move(hook));
}

bool HookManager::initialize(JNIEnv* env) {
    for (auto& hook : javaHooks) {
        if (!hook->install(env)) {
            return false;
        }
    }
    return true;
}