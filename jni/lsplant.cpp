#include "lsplant.hpp"

namespace lsplant {

    int InitializeInternal(JNIEnv* env) {
        // Dummy init, tidak melakukan apa-apa
        (void)env;
        return LSPLANT_SUCCESS;
    }

    bool Hook(JNIEnv* env,
              jclass clazz,
              const char* methodName,
              const char* methodSignature,
              void* replacement,
              void** original) {
        // Dummy hook: anggap hook selalu berhasil
        (void)env;
        (void)clazz;
        (void)methodName;
        (void)methodSignature;
        (void)replacement;
        (void)original;
        return true;
    }
}