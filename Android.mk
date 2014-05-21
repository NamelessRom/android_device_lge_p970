ifneq ($(filter p970,$(TARGET_DEVICE)),)
include $(call first-makefiles-under,$(call my-dir))
endif
