#include "hook.hpp"
#include "zygisk.hpp"

void HookManager::addJavaHook(const std::shared_ptr<JavaMethodHook>& hook) {
    javaHooks.push_back(hook);
}

void HookManager::applyJavaHooks(JNIEnv* env) {
    for (const auto& hook : javaHooks) {
        hook->apply(env);
    }
}

bool JavaMethodHook::apply(JNIEnv* env) {
    jclass clazz = env->FindClass(className.c_str());
    if (!clazz) {
        Logger::error("Class not found: " + className);
        return false;
    }

    jmethodID method = env->GetMethodID(clazz, methodName.c_str(), methodSignature.c_str());
    if (!method) {
        Logger::error("Method not found: " + methodName + " " + methodSignature);
        return false;
    }

    Logger::info("Applying hook to " + className + "." + methodName);

    int result = lsplant::InitializeInternal(env);
    if (result != LSPLANT_SUCCESS) {
        Logger::error("lsplant initialization failed");
        return false;
    }

    if (DobbyHook((void*)method, replacement, original) != RS_SUCCESS) {
        Logger::error("Failed to hook method: " + methodName);
        return false;
    }

    Logger::info("Hook applied: " + className + "." + methodName);
    return true;
}