#include "hook.hpp"
#include <string>
#include <vector>
#include <dobby.h>
#include <thread>
#include <chrono>

// Initialize the hook manager
bool HookManager::initialize(JNIEnv* env) {
    this->env = env;
    
    // Initialize LSPlant
    if (!initializeLSPlant()) {
        Logger::error("Failed to initialize LSPlant");
        return false;
    }
    
    Logger::info("HookManager initialized successfully");
    return true;
}

// Initialize LSPlant for Java method hooking
bool HookManager::initializeLSPlant() {
    return lsplant::InitializeInternal(env);
}

// Apply all hooks
bool HookManager::applyHooks() {
    bool success = true;
    
    // Hook Java methods
    if (!hookFingerprintJavaMethods()) {
        Logger::error("Failed to hook fingerprint Java methods");
        success = false;
    }
    
    // Hook native methods
    if (!hookFingerprintNativeMethods()) {
        Logger::error("Failed to hook fingerprint native methods");
        success = false;
    }
    
    if (success) {
        Logger::info("All hooks applied successfully");
    }
    
    return success;
}

// Remove all hooks
bool HookManager::removeHooks() {
    bool success = true;
    
    // Remove Java hooks
    for (auto& hook : javaHooks) {
        if (!hook->remove()) {
            Logger::error("Failed to remove Java hook: " + hook->getClassName() + "." + hook->getMethodName());
            success = false;
        }
    }
    javaHooks.clear();
    
    // Remove native hooks
    for (auto& hook : nativeHooks) {
        if (!hook->remove()) {
            Logger::error("Failed to remove native hook");
            success = false;
        }
    }
    nativeHooks.clear();
    
    if (success) {
        Logger::info("All hooks removed successfully");
    }
    
    return success;
}

// Check if a package should be hooked
bool HookManager::shouldHookPackage(const std::string& packageName) {
    for (const auto& pkg : targetPackages) {
        if (packageName.find(pkg) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Get JNI environment
JNIEnv* HookManager::getEnv() const {
    return env;
}

// Destructor
HookManager::~HookManager() {
    removeHooks();
}

// Implementation of Java method hook
JavaMethodHook::JavaMethodHook(const std::string& className, const std::string& methodName, 
                             const std::string& methodSignature, 
                             const std::function<jobject(JNIEnv*, jclass, jobject, jobjectArray)>& replacement)
    : className(className), methodName(methodName), methodSignature(methodSignature), replacement(replacement) {
}

JavaMethodHook::~JavaMethodHook() {
    remove();
}

bool JavaMethodHook::apply(JNIEnv* env) {
    // Find class
    jclass clazz = env->FindClass(className.c_str());
    if (!clazz) {
        Logger::error("Failed to find class: " + className);
        return false;
    }
    
    // Hook method
    auto result = lsplant::Hook(env, clazz, methodName.c_str(), methodSignature.c_str(), replacement, &originalMethod);
    if (!result) {
        Logger::error("Failed to hook method: " + className + "." + methodName);
        return false;
    }
    
    hooked = true;
    Logger::info("Successfully hooked method: " + className + "." + methodName);
    return true;
}

bool JavaMethodHook::remove() {
    if (!hooked) {
        return true;
    }
    
    // TODO: Implement unhook if needed
    // LSPlant currently doesn't support unhooking
    
    hooked = false;
    return true;
}

// Implementation of Native method hook
NativeMethodHook::NativeMethodHook(void* targetAddr, void* replacementAddr, void** originalAddr)
    : targetAddr(targetAddr), replacementAddr(replacementAddr), originalAddr(originalAddr) {
}

NativeMethodHook::~NativeMethodHook() {
    remove();
}

bool NativeMethodHook::apply() {
    if (DobbyHook(targetAddr, replacementAddr, originalAddr) != 0) {
        Logger::error("Failed to hook native method");
        return false;
    }
    
    hooked = true;
    Logger::info("Successfully hooked native method");
    return true;
}

bool NativeMethodHook::remove() {
    if (!hooked) {
        return true;
    }
    
    if (DobbyDestroy(targetAddr) != 0) {
        Logger::error("Failed to unhook native method");
        return false;
    }
    
    hooked = false;
    return true;
}

// Hook fingerprint-related Java methods
bool HookManager::hookFingerprintJavaMethods() {
    if (config.isMethodBypassed("isHardwareDetectedHyperOS")) {
        auto isHardwareDetectedStubHook = std::make_unique<JavaMethodHook>(
            "com/android/server/biometrics/fingerprint/FingerprintAuthenticator",
            "isHardwareDetected",
            "(Ljava/lang/String;)Z",
            [](JNIEnv* env, jclass, jobject thiz, jstring opPackageName) -> jobject {
                Logger::debug("FingerprintAuthenticator.isHardwareDetected (HyperOS) hooked, returning true");
                jboolean result = JNI_TRUE;
                return env->NewObject(env->FindClass("java/lang/Boolean"),
                                      env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"),
                                      result);
            });
        if (!isHardwareDetectedStubHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintAuthenticator.isHardwareDetected");
        }
        HookManager::getInstance().addJavaHook(std::move(isHardwareDetectedStubHook));
    }
    
    auto& config = Configuration::getInstance();
    
    // Hook FingerprintManager and FingerprintService methods
    
    // 1. isHardwareDetected
    if (config.isMethodBypassed("isHardwareDetected")) {
        // First hook the standard Android API
        auto isHardwareDetectedHook = std::make_unique<JavaMethodHook>(
            "android/hardware/fingerprint/FingerprintManager", 
            "isHardwareDetected", 
            "()Z",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("FingerprintManager.isHardwareDetected hooked by mutz, returning true");
                jboolean result = JNI_TRUE;
                return env->NewObject(env->FindClass("java/lang/Boolean"), 
                                    env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"), 
                                    result);
            }
        );
        
        if (!isHardwareDetectedHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintManager.isHardwareDetected");
        } else {
            javaHooks.push_back(std::move(isHardwareDetectedHook));
            Logger::info("Successfully hooked FingerprintManager.isHardwareDetected");
        }
        
        // HyperOS 2 specific - Hook FingerprintServiceStubImpl which is causing the errors in the log
        auto fingerprintServiceHook = std::make_unique<JavaMethodHook>(
            "com/android/server/biometrics/fingerprint/FingerprintServiceStubImpl", 
            "isHardwareDetected", 
            "()Z",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("FingerprintServiceStubImpl.isHardwareDetected hooked by mutz, returning true");
                jboolean result = JNI_TRUE;
                return env->NewObject(env->FindClass("java/lang/Boolean"), 
                                    env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"), 
                                    result);
            }
        );
        
        if (!fingerprintServiceHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintServiceStubImpl.isHardwareDetected");
            
            // Try alternative class paths for HyperOS
            auto alternativeHook = std::make_unique<JavaMethodHook>(
                "com/android/server/biometrics/sensors/fingerprint/FingerprintServiceStubImpl", 
                "isHardwareDetected", 
                "()Z",
                [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                    Logger::debug("Alternative FingerprintServiceStubImpl.isHardwareDetected hooked by mutz, returning true");
                    jboolean result = JNI_TRUE;
                    return env->NewObject(env->FindClass("java/lang/Boolean"), 
                                        env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"), 
                                        result);
                }
            );
            
            if (!alternativeHook->apply(env)) {
                Logger::warn("Failed to hook alternative FingerprintServiceStubImpl.isHardwareDetected");
            } else {
                javaHooks.push_back(std::move(alternativeHook));
                Logger::info("Successfully hooked alternative FingerprintServiceStubImpl.isHardwareDetected");
            }
        } else {
            javaHooks.push_back(std::move(fingerprintServiceHook));
            Logger::info("Successfully hooked FingerprintServiceStubImpl.isHardwareDetected");
        }
        
        // Hook additional HyperOS specific classes
        if (config.getBypassMode() == BypassMode::FULL || 
            config.getBypassMode() == BypassMode::AUTHENTICATION || 
            config.getCustomParameter("hyperos_mode") == "true") {
            
            // Try hooking Xiaomi/HyperOS specific fingerprint classes
            std::vector<std::string> hyperosClasses = {
                "com/xiaomi/biometric/fingerprint/FingerprintManager",
                "com/android/server/biometrics/BiometricServiceBase",
                "com/android/server/biometrics/BiometricUtils",
                "com/android/server/biometrics/fingerprint/FingerprintUtils",
                "com/mi/android/server/MiBiometricService"
            };
            
            for (const auto& className : hyperosClasses) {
                auto hyperosHook = std::make_unique<JavaMethodHook>(
                    className.c_str(), 
                    "isHardwareDetected", 
                    "()Z",
                    [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                        Logger::debug("HyperOS class isHardwareDetected hooked by mutz, returning true");
                        jboolean result = JNI_TRUE;
                        return env->NewObject(env->FindClass("java/lang/Boolean"), 
                                            env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"), 
                                            result);
                    }
                );
                
                if (!hyperosHook->apply(env)) {
                    Logger::debug("Failed to hook HyperOS class: " + className);
                } else {
                    javaHooks.push_back(std::move(hyperosHook));
                    Logger::info("Successfully hooked HyperOS class: " + className);
                }
            }
        }
    }
    
    // 2. canAuthenticate
    if (config.isMethodBypassed("canAuthenticate")) {
        auto canAuthenticateHook = std::make_unique<JavaMethodHook>(
            "android/hardware/fingerprint/FingerprintManager", 
            "canAuthenticate", 
            "()I",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("canAuthenticate hooked, returning BIOMETRIC_SUCCESS");
                jint result = 0; // BIOMETRIC_SUCCESS = 0
                return env->NewObject(env->FindClass("java/lang/Integer"), 
                                    env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), 
                                    result);
            }
        );
        
        if (!canAuthenticateHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintManager.canAuthenticate");
        } else {
            javaHooks.push_back(std::move(canAuthenticateHook));
        }
    }
    
    // 3. hasEnrolledFingerprints
    if (config.isMethodBypassed("hasEnrolledFingerprints")) {
        auto hasEnrolledFingerprintsHook = std::make_unique<JavaMethodHook>(
            "android/hardware/fingerprint/FingerprintManager", 
            "hasEnrolledFingerprints", 
            "()Z",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("hasEnrolledFingerprints hooked, returning true");
                jboolean result = JNI_TRUE;
                return env->NewObject(env->FindClass("java/lang/Boolean"), 
                                    env->GetMethodID(env->FindClass("java/lang/Boolean"), "<init>", "(Z)V"), 
                                    result);
            }
        );
        
        if (!hasEnrolledFingerprintsHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintManager.hasEnrolledFingerprints");
        } else {
            javaHooks.push_back(std::move(hasEnrolledFingerprintsHook));
        }
    }
    
    // 4. getAuthenticatorId
    if (config.isMethodBypassed("getAuthenticatorId")) {
        auto getAuthenticatorIdHook = std::make_unique<JavaMethodHook>(
            "android/hardware/fingerprint/FingerprintManager", 
            "getAuthenticatorId", 
            "()J",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("getAuthenticatorId hooked, returning fake ID");
                jlong result = 0x1234567890ABCDEF; // Fake authenticator ID
                return env->NewObject(env->FindClass("java/lang/Long"), 
                                    env->GetMethodID(env->FindClass("java/lang/Long"), "<init>", "(J)V"), 
                                    result);
            }
        );
        
        if (!getAuthenticatorIdHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintManager.getAuthenticatorId");
        } else {
            javaHooks.push_back(std::move(getAuthenticatorIdHook));
        }
    }
    
    // 5. authenticate
    if (config.isMethodBypassed("authenticate")) {
        auto authenticateHook = std::make_unique<JavaMethodHook>(
            "android/hardware/fingerprint/FingerprintManager", 
            "authenticate", 
            "(Landroid/hardware/fingerprint/FingerprintManager$CryptoObject;Landroid/os/CancellationSignal;Landroid/hardware/fingerprint/FingerprintManager$AuthenticationCallback;Landroid/os/Handler;I)V",
            [](JNIEnv* env, jclass, jobject thiz, jobjectArray args) -> jobject {
                Logger::debug("authenticate hooked, auto-authenticating after delay");
                
                // Wait for the main thread to be ready
                std::thread([env, args]() {
                    // Sleep for a short time to make it look realistic
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    
                    // Get the callback object
                    jobject callback = env->GetObjectArrayElement(args, 2);
                    
                    // Create a fake fingerprint authentication result
                    jclass authResultClass = env->FindClass("android/hardware/fingerprint/FingerprintManager$AuthenticationResult");
                    jmethodID authResultConstructor = env->GetMethodID(authResultClass, "<init>", "(Landroid/hardware/fingerprint/FingerprintManager$CryptoObject;Landroid/hardware/fingerprint/FingerprintManager$AuthenticationResult;I)V");
                    
                    // Create fake result
                    jobject cryptoObject = nullptr; // No crypto object
                    jobject authResult = nullptr;   // No authentication result
                    jint userId = 0;               // Default user ID
                    
                    jobject result = env->NewObject(authResultClass, authResultConstructor, cryptoObject, authResult, userId);
                    
                    // Call onAuthenticationSucceeded
                    jclass callbackClass = env->GetObjectClass(callback);
                    jmethodID onSuccessMethod = env->GetMethodID(callbackClass, "onAuthenticationSucceeded", "(Landroid/hardware/fingerprint/FingerprintManager$AuthenticationResult;)V");
                    env->CallVoidMethod(callback, onSuccessMethod, result);
                    
                    // Cleanup
                    env->DeleteLocalRef(callback);
                    env->DeleteLocalRef(authResultClass);
                    env->DeleteLocalRef(callbackClass);
                    env->DeleteLocalRef(result);
                }).detach();
                
                return nullptr; // void method
            }
        );
        
        if (!authenticateHook->apply(env)) {
            Logger::warn("Failed to hook FingerprintManager.authenticate");
        } else {
            javaHooks.push_back(std::move(authenticateHook));
        }
    }
    
    return true;
}

// Hook fingerprint-related native methods
bool HookManager::hookFingerprintNativeMethods() {
    auto& config = Configuration::getInstance();
    
    // Check if we need HyperOS specific handling
    bool isHyperOSMode = config.getCustomParameter("hyperos_mode") == "true";
    
    if (isHyperOSMode) {
        Logger::info("HyperOS native method hooking activated by mutz");
        
        // Try hooking known HyperOS/Xiaomi native methods related to fingerprint hardware detection
        // Note: This is a best-effort approach since we don't have full library analysis
        
        std::vector<std::string> nativeLibraries = {
            "/system/lib64/libMiuiFingerprintHal.so",
            "/system/lib64/libfingerprint_client.so",
            "/system/lib64/libfingerprintservice.so",
            "/system/lib64/hw/fingerprint.default.so"
        };
        
        // Try to find these libraries and then hook their isHardwareDetected equivalent functions
        // This is a simplified version - in real implementation we'd need to find exact offsets
        for (const auto& libPath : nativeLibraries) {
            if (access(libPath.c_str(), F_OK) == 0) {
                Logger::info("Found native library: " + libPath);
                
                // In a real implementation, we would find offsets to hardware detection functions
                // and hook them here. For now, just log that we found the library.
                
                // Example of what real hooking would look like (pseudo-code):
                // void* libHandle = dlopen(libPath.c_str(), RTLD_NOW);
                // if (libHandle) {
                //     void* funcAddr = dlsym(libHandle, "isHardwareDetected");
                //     if (funcAddr) {
                //         // Create and apply hook
                //     }
                // }
            }
        }
    }
    
    return true;
}
