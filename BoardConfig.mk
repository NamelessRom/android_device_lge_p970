# inherit from the proprietary version
-include vendor/lge/p970/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := omap3

# Board configuration
#
TARGET_ARCH                := arm
TARGET_NO_BOOTLOADER       := true
TARGET_CPU_ABI             := armeabi-v7a
TARGET_CPU_ABI2            := armeabi
TARGET_ARCH_VARIANT        := armv7-a-neon
TARGET_ARCH_VARIANT_CPU    := cortex-a8
TARGET_ARCH_VARIANT_FPU    := neon
TARGET_CPU_VARIANT         := cortex-a8
TARGET_ARCH_LOWMEM         := true
ARCH_ARM_HAVE_ARMV7A       := true
TARGET_ARCH_HAVE_NEON      := true
ARCH_ARM_HAVE_TLS_REGISTER := true

TARGET_NO_BOOTLOADER           := true
TARGET_BOOTLOADER_BOARD_NAME   := black
TARGET_PROVIDES_INIT_TARGET_RC := true

TARGET_GCC_VERSION_EXP         := 4.8
TARGET_KERNEL_CUSTOM_TOOLCHAIN := arm-eabi-4.7

TARGET_GLOBAL_CFLAGS   += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp

BOARD_KERNEL_CMDLINE := androidboot.selinux=permissive
BOARD_KERNEL_BASE    := 0x80000000
BOARD_PAGE_SIZE      := 0x00000800

TARGET_OMAP3         := true
OMAP_ENHANCEMENT     := true
ifdef OMAP_ENHANCEMENT
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP3 -DOMAP_ENHANCEMENT_CPCAM -DOMAP_ENHANCEMENT_VTC
endif

# Kernel
TARGET_KERNEL_SOURCE        := kernel/lge/p970
TARGET_KERNEL_CONFIG        := custom_p970_defconfig
TARGET_RECOVERY_INITRC      := device/lge/p970/recovery/init-recovery.rc
TARGET_SPECIFIC_HEADER_PATH := device/lge/p970/include

TARGET_USERIMAGES_USE_EXT4         := true
TARGET_USERIMAGES_USE_F2FS         := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 665681920
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1170259968
BOARD_FLASH_BLOCK_SIZE             := 131072

BOARD_CUSTOM_GRAPHICS         := ../../../device/lge/p970/recovery/graphics.c
TARGET_RECOVERY_PIXEL_FORMAT  := "RGBX_8888"
TARGET_RECOVERY_FSTAB         := device/lge/p970/rootdir/etc/fstab.black
RECOVERY_FSTAB_VERSION        := 2

BOARD_HAS_NO_SELECT_BUTTON := true

BOARD_HAVE_BLUETOOTH                        := true
BOARD_HAVE_BLUETOOTH_BCM                    := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/lge/p970/bluetooth
BOARD_BLUEDROID_VENDOR_CONF                 := device/lge/p970/bluetooth/vnd_bt.txt
#BOARD_HAVE_FM_RADIO                         := true

# ION
BOARD_HAVE_OLD_ION_API := true

# BOARD_USES_TI_CAMERA_HAL := true
HARDWARE_OMX := true
ifdef HARDWARE_OMX
OMX_JPEG                  := true
OMX_VENDOR                := ti
TARGET_USE_OMX_RECOVERY   := true
TARGET_USE_OMAP_COMPAT    := true
BUILD_WITH_TI_AUDIO       := 1
BUILD_PV_VIDEO_ENCODERS   := 1
OMX_VENDOR_INCLUDES := \
   hardware/ti/omap3/omx/system/src/openmax_il/omx_core/inc \
   hardware/ti/omap3/omx/image/src/openmax_il/jpeg_enc/inc
OMX_VENDOR_WRAPPER       := TI_OMX_Wrapper
BOARD_OPENCORE_LIBRARIES := libOMX_Core
BOARD_OPENCORE_FLAGS     := -DHARDWARE_OMX=1
#BOARD_CAMERA_LIBRARIES   := libcamera
endif

BOARD_MOBILEDATA_INTERFACE_NAME := "vsnet0"

BOARD_WLAN_DEVICE                := bcmdhd
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_bcmdhd
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
WPA_SUPPLICANT_VERSION           := VER_0_8_X
WIFI_DRIVER_FW_PATH_PARAM        := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA          := "/system/vendor/firmware/fw_bcmdhd.bin"
WIFI_DRIVER_FW_PATH_P2P          := "/system/vendor/firmware/fw_bcmdhd.bin"
WIFI_DRIVER_FW_PATH_AP           := "/system/vendor/firmware/fw_bcmdhd_apsta.bin"
BOARD_LEGACY_NL80211_STA_EVENTS  := true
#WIFI_DRIVER_HAS_LGE_SOFTAP       := true

BOARD_EGL_CFG       := device/lge/p970/configs/egl.cfg
ENABLE_WEBGL        := true
USE_OPENGL_RENDERER := true

COMMON_GLOBAL_CFLAGS += -DSYSTEMUI_PBSIZE_HACK=1
COMMON_GLOBAL_CFLAGS += -DWORKAROUND_BUG_10194508=1

BOARD_HAS_VIBRATOR_IMPLEMENTATION := ../../device/lge/p970/vibrator.c
BOARD_SYSFS_LIGHT_SENSOR          := "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-0060/leds/lcd-backlight/als"

COMMON_GLOBAL_CFLAGS           += -DNEEDS_VECTORIMPL_SYMBOLS -DHAS_CONTEXT_PRIORITY -DDONT_USE_FENCE_SYNC
COMMON_GLOBAL_CFLAGS           += -DBOARD_CHARGING_CMDLINE_NAME='"rs"' -DBOARD_CHARGING_CMDLINE_VALUE='"c"'
BOARD_ALLOW_SUSPEND_IN_CHARGER := true

## Radio fixes
BOARD_RIL_CLASS := ../../../device/lge/p970/ril/

# Audio
BOARD_HAVE_PRE_KITKAT_AUDIO_BLOB := true
COMMON_GLOBAL_CFLAGS             += -DICS_AUDIO_BLOB -DAUDIO_LEGACY_HACK -DHAVE_PRE_KITKAT_AUDIO_BLOB

# Camera
COMMON_GLOBAL_CFLAGS += -DICS_CAMERA_BLOB -DOMAP_ICS_CAMERA -DCAMERA_LEGACY_HACK

# Use cm powerhal
TARGET_POWERHAL_VARIANT    := cm
TARGET_USES_CPU_BOOST_HINT := true

# Fix Graphics Issues
PRODUCT_PROPERTY_OVERRIDES += \
        ro.zygote.disable_gl_preload=true \
        ro.bq.gpu_to_cpu_unsupported=true \
        dalvik.vm.debug.alloc=0 \
        ro.hwui.disable_scissor_opt=true

# No, we dont want METADATA -.-
SKIP_SET_METADATA := true

# Additional Props
PRODUCT_PROPERTY_OVERRIDES += \
        ro.config.low_ram=true \
        dalvik.vm.jit.codecachesize=0

# adb root
ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=0 \
        ro.secure=0

# Misc Flags
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/omap/musb-omap2430/musb-hdrc/gadget/lun%d/file"
BOARD_SKIP_ANDROID_DOC_BUILD := true
