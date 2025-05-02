LOCAL_PATH := $(call my-dir)

# Modul utama
include $(CLEAR_VARS)
LOCAL_MODULE := FingerprintBypassModule
LOCAL_SRC_FILES := main.cpp hook.cpp config.cpp module.cpp
LOCAL_STATIC_LIBRARIES := libdobby
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -std=c++20 -Wall -Wextra -fno-rtti -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++20
include $(BUILD_SHARED_LIBRARY)

# Dobby library
include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)
