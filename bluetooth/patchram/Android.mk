LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := brcm_patchram_plus
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := brcm_patchram_plus.c

LOCAL_CFLAGS := -DLOG_TAG=\"brcm_patchram_plus\"

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_BIN)
LOCAL_STATIC_LIBRARIES := liblog libcutils libc

LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)
