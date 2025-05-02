#pragma once

#include <jni.h>

#define LSPLANT_SUCCESS 0
#define LSPLANT_FAILURE -1

namespace lsplant {

    // Inisialisasi internal LSPlant, misalnya untuk cache env atau set up
    int InitializeInternal(JNIEnv* env);

    // Hook Java method dengan LSPlant
    bool Hook(JNIEnv* env,
              jclass clazz,
              const char* methodName,
              const char* methodSignature,
              void* replacement,
              void** original);
}