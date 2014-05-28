# Boot animation
PRODUCT_NO_BOOTANIMATION := true
TARGET_SCREEN_HEIGHT := 800
TARGET_SCREEN_WIDTH := 480

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

# Inherit wifi broadcom stuffs
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4329/device-bcm.mk)
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/config/config-bcm.mk)

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
    $(LOCAL_PATH)/audio/asound.conf:system/etc/asound.conf

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
    $(LOCAL_PATH)/wifi/nvram.txt:system/etc/wifi/nvram.txt \
    $(LOCAL_PATH)/wifi/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    $(LOCAL_PATH)/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

# Touchscreen configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/hub_synaptics_touch.kl:system/usr/keylayout/hub_synaptics_touch.kl \
    $(LOCAL_PATH)/configs/hub_synaptics_touch.idc:system/usr/idc/hub_synaptics_touch.idc

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/Hookkey.kl:system/usr/keylayout/Hookkey.kl

# Charger mode
PRODUCT_PACKAGES += \
    charger \
    charger_res_images

PRODUCT_PACKAGES += \
    power.black \
    prb \
    lgcpversion \
    brcm_patchram_plus

# Wifi
PRODUCT_PACKAGES += \
    wifimac \
    dhdutil \
    libnetcmdiface

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

PRODUCT_PACKAGES += \
    lights.black \
    audio.a2dp.default \
    audio_policy.default \
    hwcomposer.black \
    libaudioutils \
    libtiutils \
    libion_ti \
    libomap_mm_library_jni \
    libemoji
#    camera.omap3

# legacy version of skia
# fixes the app switcher previews
PRODUCT_PACKAGES += \
    libskia_legacy

# F2FS filesystem
PRODUCT_PACKAGES += \
    mkfs.f2fs \
    fsck.f2fs \
    fibmap.f2fs \
    f2fstat

$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

PRODUCT_AAPT_CONFIG := normal hdpi

PRODUCT_NAME := nameless_p970
PRODUCT_RELEASE_NAME := OptimusBlack
PRODUCT_DEVICE := p970
PRODUCT_BRAND := LGE
PRODUCT_MANUFACTURER := LGE
PRODUCT_MODEL := LG-P970
