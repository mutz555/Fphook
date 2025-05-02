LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FingerprintBypassModule
LOCAL_SRC_FILES := main.cpp hook.cpp config.cpp module.cpp
LOCAL_STATIC_LIBRARIES := libdobby libcxx_static 
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -std=c++20 -Wall -Wextra -fno-rtti -fvisibility=hidden
LOCAL_CPPFLAGS := -std=c++20
include $(BUILD_SHARED_LIBRARY)

# Dobby library
include $(CLEAR_VARS)
LOCAL_MODULE := dobby
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

# LSPlant library
include $(CLEAR_VARS)
LOCAL_MODULE := lsplant
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/liblsplant.a
include $(PREBUILT_STATIC_LIBRARY)
