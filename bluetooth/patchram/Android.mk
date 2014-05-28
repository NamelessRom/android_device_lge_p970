LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := liblog libcutils libc

LOCAL_SRC_FILES := brcm_patchram_plus.c

LOCAL_CFLAGS := -DLOG_TAG=\"brcm_patchram_plus\"

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE := brcm_patchram_plus
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

include $(BUILD_EXECUTABLE)
