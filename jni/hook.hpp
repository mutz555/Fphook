#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <dobby.h>
#include <lsplant.hpp>
#include "logging.hpp"
#include "config.hpp"

// Forward declarations
class JavaMethodHook;
class NativeMethodHook;

// Singleton Hook Manager
class HookManager {
public:
    static HookManager& getInstance() {
        static HookManager instance;
        return instance;
    }
    
    // Initialize the hook manager with JNI environment
    bool initialize(JNIEnv* env);
    
    // Apply all hooks
    bool applyHooks();
    
    // Remove all hooks
    bool removeHooks();
    
    // Check if specific package should be hooked
    bool shouldHookPackage(const std::string& packageName);
    
    // Get JNI environment
    JNIEnv* getEnv() const;
    
private:
    HookManager() = default;
    ~HookManager();
    
    // Disallow copy/move
    HookManager(const HookManager&) = delete;
    HookManager& operator=(const HookManager&) = delete;
    HookManager(HookManager&&) = delete;
    HookManager& operator=(HookManager&&) = delete;
    
    // Initialize LSPlant for Java hooking
    bool initializeLSPlant();
    
    // Hook fingerprint-related Java methods
    bool hookFingerprintJavaMethods();
    
    // Hook fingerprint-related native methods
    bool hookFingerprintNativeMethods();
    
    JNIEnv* env = nullptr;
    std::vector<std::unique_ptr<JavaMethodHook>> javaHooks;
    std::vector<std::unique_ptr<NativeMethodHook>> nativeHooks;
    
    // Target packages to hook
    std::vector<std::string> targetPackages = {
        "android", 
        "com.android.systemui", 
        "com.android.settings",
        "com.xiaomi.settings",
        "com.infinix.settings"
    };
};

// Java method hook class
class JavaMethodHook {
public:
    JavaMethodHook(const std::string& className, const std::string& methodName, 
                   const std::string& methodSignature, 
                   const std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)>& replacement);
    ~JavaMethodHook();
    
    bool apply(JNIEnv* env);
    bool remove();
    
    const std::string& getClassName() const { return className; }
    const std::string& getMethodName() const { return methodName; }
    
private:
    std::string className;
    std::string methodName;
    std::string methodSignature;
    std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)> replacement;
    void* originalMethod = nullptr;
    bool hooked = false;
};

// Native method hook class
class NativeMethodHook {
public:
    NativeMethodHook(void* targetAddr, void* replacementAddr, void** originalAddr);
    ~NativeMethodHook();
    
    bool apply();
    bool remove();
    
private:
    void* targetAddr;
    void* replacementAddr;
    void** originalAddr;
    bool hooked = false;
};

// Helper functions for hooking
jboolean JNICALL isHardwareDetected_hook(JNIEnv* env, jobject thiz);
jboolean JNICALL canAuthenticate_hook(JNIEnv* env, jobject thiz);
jboolean JNICALL hasEnrolledFingerprints_hook(JNIEnv* env, jobject thiz);
jlong JNICALL getAuthenticatorId_hook(JNIEnv* env, jobject thiz);
jint JNICALL authenticate_hook(JNIEnv* env, jobject thiz, jobject crypto, jobject cancel, jobject callback, jint flags);
