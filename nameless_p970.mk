# Boot animation
PRODUCT_NO_BOOTANIMATION := true
TARGET_SCREEN_HEIGHT := 800
TARGET_SCREEN_WIDTH := 480

# do not include DSPManager
USE_DSP_MANAGER := false

$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_us_supl.mk)

# Get the prebuilt list of APNs
$(call inherit-product, vendor/nameless/config/apns.mk)

# Inherit from the common Open Source product configuration
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from our custom product configuration
$(call inherit-product, vendor/nameless/config/common.mk)

$(call inherit-product-if-exists, vendor/lge/p970/p970-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/lge/p970/overlay

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/recovery/postrecoveryboot.sh:recovery/root/sbin/postrecoveryboot.sh

# Ramdisk
PRODUCT_PACKAGES += \
    fstab.black \
    init.black.rc \
    init.black.usb.rc \
    ueventd.black.rc

# Alsa configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_policy.conf:system/etc/audio_policy.conf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media/media_profiles.xml:system/etc/media_profiles.xml \
    $(LOCAL_PATH)/media/media_codecs.xml:system/etc/media_codecs.xml

# init.d scripts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilt/etc/init.d/10app2sd:system/etc/init.d/10app2sd

# Permission files
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml

# RIL stuffs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ipc_channels.config:system/etc/ipc_channels.config \
    $(LOCAL_PATH)/prebuilt/bin/init.vsnet:system/bin/init.vsnet \
    $(LOCAL_PATH)/prebuilt/bin/init.vsnet-down:system/bin/init.vsnet-down

# GPS
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gps_brcm_conf.xml:system/etc/gps_brcm_conf.xml

# Wifi
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wifi/wifimac/wlan-precheck:system/bin/wlan-precheck \
    $(LOCAL_PATH)/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    $(LOCAL_PATH)/wifi/nvram.txt:system/etc/wifi/nvram.txt \
    $(LOCAL_PATH)/wifi/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    $(LOCAL_PATH)/wifi/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/wifi/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

# Touchscreen configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/hub_synaptics_touch.kl:system/usr/keylayout/hub_synaptics_touch.kl \
    $(LOCAL_PATH)/configs/hub_synaptics_touch.idc:system/usr/idc/hub_synaptics_touch.idc

# Key layouts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/Generic.kl:system/usr/keylayout/Generic.kl \
    $(LOCAL_PATH)/configs/Hookkey.kl:system/usr/keylayout/Hookkey.kl \

# Charger mode
PRODUCT_PACKAGES += \
    charger \
    charger_res_images

PRODUCT_PACKAGES += \
    lgcpversion \
    prb \
    wifimac

# OMX components
PRODUCT_PACKAGES += \
    libstagefrighthw \
    libbridge \
    cexec.out \
    libPERF \
    libOMX_Core \
    libLCML \
    libOMX.TI.Video.Decoder \
    libOMX.TI.Video.encoder \
    libOMX.TI.WBAMR.decode \
    libOMX.TI.AAC.encode \
    libOMX.TI.G722.decode \
    libOMX.TI.MP3.decode \
    libOMX.TI.WMA.decode \
    libOMX.TI.Video.encoder \
    libOMX.TI.WBAMR.encode \
    libOMX.TI.G729.encode \
    libOMX.TI.AAC.decode \
    libOMX.TI.VPP \
    libOMX.TI.G711.encode \
    libOMX.TI.JPEG.encoder \
    libOMX.TI.G711.decode \
    libOMX.TI.ILBC.decode \
    libOMX.TI.ILBC.encode \
    libOMX.TI.AMR.encode \
    libOMX.TI.G722.encode \
    libOMX.TI.JPEG.decoder \
    libOMX.TI.G726.encode \
    libOMX.TI.G729.decode \
    libOMX.TI.Video.Decoder \
    libOMX.TI.AMR.decode \
    libOMX.TI.G726.decode

# ITTIAM OMX
PRODUCT_PACKAGES += \
    libOMX.TI.720P.Decoder \
    libOMX.TI.720P.Encoder

# OMAP3
PRODUCT_PACKAGES += \
    hwcomposer.omap3 \
    power.omap3
#    camera.omap3

PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio_policy.default \
    audio.primary.black \
    audio.r_submix.default \
    audio.usb.default \
    libaudioutils \
    libemoji \
    libion_ti \
    libomap_mm_library_jni \
    libtiutils \
    lights.black

$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

# Fix Graphics Issues
PRODUCT_PROPERTY_OVERRIDES += \
        ro.zygote.disable_gl_preload=true \
        ro.bq.gpu_to_cpu_unsupported=true \
        dalvik.vm.debug.alloc=0 \
        ro.hwui.disable_scissor_opt=true

# Additional hwui configuration
PRODUCT_PROPERTY_OVERRIDES += \
        hwui.use_gpu_pixel_buffers=false \
        ro.hwui.drop_shadow_cache_size=0.5 \
        ro.hwui.fbo_cache_size=4 \
        ro.hwui.r_buffer_cache_size=1 \
        ro.hwui.text_gamma_correction=shader \
        ro.hwui.text_large_cache_width=512 \
        ro.hwui.text_large_cache_height=128 \
        ro.hwui.text_small_cache_width=256 \
        ro.hwui.text_small_cache_height=64 \
        ro.hwui.texture_cache_size=4 \

# Additional Props
PRODUCT_PROPERTY_OVERRIDES += \
        ro.config.low_ram=true \
        dalvik.vm.jit.codecachesize=0

# adb root
ADDITIONAL_DEFAULT_PROPERTIES += \
        ro.adb.secure=0 \
        ro.secure=0

# Enable Torch
PRODUCT_PACKAGES += Torch

PRODUCT_AAPT_CONFIG := normal hdpi

PRODUCT_NAME := nameless_p970
PRODUCT_RELEASE_NAME := OptimusBlack
PRODUCT_DEVICE := p970
PRODUCT_BRAND := LGE
PRODUCT_MANUFACTURER := LGE
PRODUCT_MODEL := LG-P970
