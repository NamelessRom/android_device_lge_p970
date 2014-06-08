/*
 * Copyright (C) 2011 The CyanogenMod Project
 * Copyright (C) 2014 Stefan Demharter <stefan.demharter@gmx.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "lights"

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

#define LCD_FILE "/sys/class/leds/lcd-backlight/brightness"
#define LCD_STATE "/sys/class/leds/lcd-backlight/onoff"

#define LED_PATH "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/"

enum CYCLE {
	CYCLE_131_MS = 0,
	CYCLE_0_52_S = 1,
	CYCLE_1_05_S = 2,
	CYCLE_2_10_S = 3,
	CYCLE_4_19_S = 4,
	CYCLE_8_39_S = 5,
	CYCLE_12_6_S = 6,
	CYCLE_16_8_S = 7,
};

static const char *cycle_str[] = {
	[CYCLE_131_MS] = "131 ms",
	[CYCLE_0_52_S] = "0.52 s",
	[CYCLE_1_05_S] = "1.05 s",
	[CYCLE_2_10_S] = "2.10 s",
	[CYCLE_4_19_S] = "4.19 s",
	[CYCLE_8_39_S] = "8.39 s",
	[CYCLE_12_6_S] = "12.6 s",
	[CYCLE_16_8_S] = "16.8 s",
};

enum WAVE {
	WAVE_17 = 0,		// 12222222
	WAVE_26 = 1,		// 11222222
	WAVE_35 = 2,		// 11122222
	WAVE_44 = 3,		// 11112222
	WAVE_53 = 4,		// 11111222
	WAVE_62 = 5,		// 11111122
	WAVE_71 = 6,		// 11111112
	WAVE_8 = 7,		// 11111111
	WAVE_224 = 8,		// 11221111
	WAVE_422 = 9,		// 11112211
	WAVE_12221 = 10,	// 12211221
	WAVE_2222 = 11,		// 11221122
	WAVE_143 = 12,		// 12222111
	WAVE_242 = 13,		// 11222211
	WAVE_351 = 14,		// 11122221
	WAVE_11111111 = 15,	// 12121212 // seems to stay longer in 1 at SLOPE_4TH
};

enum SLOPE {
	SLOPE_0 = 0,
	SLOPE_16TH = 1,
	SLOPE_8TH = 2,
	SLOPE_4TH = 3,
};

enum LED {
	MENU,
	HOME,
	BACK,
	SEARCH,
	BLUELEFT,
	BLUERIGHT,
	LED_FIRST = MENU,
	LED_LAST = BLUERIGHT,
};

#define INV 256

#define LEDS 6

static int
write_string(char const* path, const char* str, int len)
{
    static int already_warned = 0;

    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        int amt = write(fd, str, len);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("liblights: write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

struct pattern {
    enum CYCLE cycle;
    enum SLOPE slope;
    enum WAVE waves[LEDS]; // from left to right, white to blue, i.e. MENU, HOME, BACK, SEARCH, BLUELEFT, BLUERIGHT
};

static struct pattern b(enum WAVE wave)
{
    struct pattern pattern = {
        .cycle = CYCLE_2_10_S,
        .slope = SLOPE_4TH,
    };
    enum LED led;
    for (led = LED_FIRST; led <= LED_LAST; ++led) {
        pattern.waves[led] = wave;
    }
    return pattern;
}

static int cycle_from_flashMS(int flashMS)
{
#define P 131
    if (flashMS <=  1 * P) return CYCLE_131_MS;
    if (flashMS <=  4 * P) return CYCLE_0_52_S;
    if (flashMS <=  8 * P) return CYCLE_1_05_S;
    if (flashMS <= 16 * P) return CYCLE_2_10_S;
    if (flashMS <= 32 * P) return CYCLE_4_19_S;
    if (flashMS <= 64 * P) return CYCLE_8_39_S;
    if (flashMS <= 96 * P) return CYCLE_12_6_S;
    return CYCLE_16_8_S;
}

static struct pattern pattern_from_light_state(struct light_state_t const* state)
{
    int flashMS = state->flashOnMS + state->flashOffMS;
    int ratio;

    ALOGD("liblights: flashMode %d\n", state->flashMode);
    ALOGD("liblights: flashMs off %d on %d\n", state->flashOffMS, state->flashOnMS);

    if (state->flashMode == LIGHT_FLASH_HARDWARE) {
        return b(WAVE_44);
    } else if (state->flashMode != LIGHT_FLASH_TIMED) {
        return b(WAVE_8 | INV);
    } else if (state->flashOnMS == 0) {
        return b(WAVE_8);
    } else if (state->flashOffMS == 0) {
        switch (state->flashOnMS) {
        case 5000:
        {
            struct pattern pattern = {
                .cycle = CYCLE_4_19_S,
                .slope = SLOPE_16TH,
                .waves = {WAVE_143, WAVE_242, WAVE_351, WAVE_44, WAVE_8 | INV, WAVE_8 | INV},
            };
            return pattern;
        }
        case 2500:
        {
            struct pattern pattern = {
                .cycle = CYCLE_2_10_S,
                .slope = SLOPE_16TH,
                .waves = {WAVE_143, WAVE_242, WAVE_351, WAVE_44, WAVE_8 | INV, WAVE_8 | INV},
            };
            return pattern;
        }
        case 1000:
        {
            struct pattern pattern = {
                .cycle = CYCLE_8_39_S,
                .slope = SLOPE_8TH,
                .waves = {WAVE_26 | INV, WAVE_224, WAVE_422, WAVE_62, WAVE_8 | INV, WAVE_8 | INV},
            };
            return pattern;
        }
        case 500:
        {
            struct pattern pattern = {
                .cycle = CYCLE_4_19_S,
                .slope = SLOPE_16TH,
                .waves = {WAVE_26, WAVE_224 | INV, WAVE_422 | INV, WAVE_62 | INV, WAVE_8 | INV, WAVE_8 | INV},
            };
            return pattern;
        }
        case 250:
        {
            struct pattern pattern = {
                .cycle = CYCLE_1_05_S,
                .waves = {WAVE_44, WAVE_44 | INV, WAVE_44 | INV, WAVE_44, WAVE_8 | INV, WAVE_8 | INV},
            };
            return pattern;
        }
        default:
            ALOGW("liblights: Unknown special mode: %d - keeping always on\n", state->flashOnMS);
        case 1: // ALWAYS ON
            return b(WAVE_8);
        }
    }
    struct pattern pattern = b(7 * state->flashOffMS / flashMS);
    pattern.cycle = cycle_from_flashMS(flashMS);
    return pattern;
}

static int
set_light_backlight(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int brightness = rgb_to_brightness(state);
    char buf[128];

    int count = snprintf(buf, 128, "%d", brightness);

    return write_string(LCD_FILE, buf, count);
}

static int
set_light_buttons(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int value[LEDS];
    int brightness = rgb_to_brightness(state);
    write_string(LED_PATH "button", brightness ? "1" : "0", 1);
    return 0;
}

static int
set_light_notifications(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int value[LEDS];
    struct pattern pattern = pattern_from_light_state(state);
    enum LED led;
    int hour;
    char pattern_string[256];
    int written;

    for (led = LED_FIRST; led <= LED_LAST; ++led) {
        value[led] = 0x7f * ((state->color >> (4 * (LEDS - 1 - led))) & 0xf) / 15;
    }

    written = snprintf(pattern_string, 256, "%d %d %d -",
                        pattern.cycle, pattern.slope, pattern.slope);

    for (led = LED_FIRST; led <= LED_LAST; ++led) {
        int inv = (unsigned int)pattern.waves[led] & INV;
        unsigned int wave = (unsigned int)pattern.waves[led] & ~INV;
        unsigned int val = value[led];
        unsigned int val0 = inv & INV ? val : 0;
        unsigned int val1 = !inv ? val : 0;
        int written2 = snprintf(pattern_string + written, 256 - written, " %u %u %u,", val0, val1, wave);

        if (written2 < 0) {
            ALOGE("liblights: Couldn't create pattern string\n");
        }
        written += written2;
    }
    write_string(LED_PATH "pattern", pattern_string, written);
    return 0;
}

/** Close the lights device */
static int
close_lights(struct hw_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

static inline int streq(const char* str1, const char* str2)
{
    return strcmp(str1, str2) == 0;
}

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (streq(LIGHT_ID_BACKLIGHT, name)) {
        set_light = set_light_backlight;
    } else if (streq(LIGHT_ID_BUTTONS, name)) {
        set_light = set_light_buttons;
    } else if (streq(LIGHT_ID_NOTIFICATIONS, name)) {
        set_light = set_light_notifications;
    } else {
        return -EINVAL;
    }

    struct light_device_t *dev = calloc(1, sizeof(struct light_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.module = module;
    dev->common.close = close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "LGE P970 lights Module",
    .author = "CyanogenMod Project",
    .methods = &lights_module_methods,
};
