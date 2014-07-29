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
    WAVE_17 = 0,        // 12222222
    WAVE_26 = 1,        // 11222222
    WAVE_35 = 2,        // 11122222
    WAVE_44 = 3,        // 11112222
    WAVE_53 = 4,        // 11111222
    WAVE_62 = 5,        // 11111122
    WAVE_71 = 6,        // 11111112
    WAVE_8 = 7,         // 11111111
    WAVE_224 = 8,       // 11221111
    WAVE_422 = 9,       // 11112211
    WAVE_12221 = 10,    // 12211221
    WAVE_2222 = 11,     // 11221122
    WAVE_143 = 12,      // 12222111
    WAVE_242 = 13,      // 11222211
    WAVE_351 = 14,      // 11122221
    WAVE_11111111 = 15, // 12121212 // seems to stay longer in 1 at SLOPE_4TH
};

enum SLOPE {
    SLOPE_0 = 0,
    SLOPE_16TH = 1,
    SLOPE_8TH = 2,
    SLOPE_4TH = 3,
};

enum KEY_LED {
    MENU,
    HOME,
    BACK,
    SEARCH,
    BLUELEFT,
    BLUERIGHT,

    KEY_LED_MIN = MENU,
    KEY_LED_MAX = BLUERIGHT,

    KEY_LED_WMIN = MENU,
    KEY_LED_WMAX = SEARCH,
    KEY_LED_BMIN = BLUELEFT,
    KEY_LED_BMAX = BLUERIGHT,
};

#define KEY_LED_WMSUM (KEY_LED_WMAX + KEY_LED_WMIN)

#define IS_WHITE(led) (led >= KEY_LED_WMIN && led <= KEY_LED_WMAX)

#define INV 256

#define LEDS (KEY_LED_MAX - KEY_LED_MIN + 1)

static int write_string(const char *path, const char *str, int len)
{
    static int already_warned = 0;

    int fd = open(path, O_WRONLY);

    if (fd >= 0) {
        int amt = write(fd, str, len);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("liblights: write_string failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int write_int(const char *path, int val)
{
    char buf[128] = {0};
    int len = snprintf(buf, sizeof(buf), "%d", val);

    return write_string(path, buf, len);
}

static int rgb_coeff_avg(int color, int r, int g, int b)
{
    return ((r * ((color >> 16) & 0xff)) \
        + (g * ((color >> 8) & 0xff)) + (b * (color & 0xff))) / (r + g + b);
}

static int rgb_to_brightness(int color)
{
    return rgb_coeff_avg(color, 77, 150, 29);
}

struct wb {
    int white;
    int blue;
};

static struct wb rgb_to_wb(int color)
{
    int white = rgb_coeff_avg(color, 87, 169, 0);
    int blue = (color & 0xff) - white;

    if (blue < 0) {
        white = rgb_coeff_avg(color, 77, 150, 29);
        blue = 0;
    }

    return (struct wb) {
        .white = white,
        .blue = blue,
    };
}

struct pattern {
    int special;
    enum CYCLE cycle;
    enum SLOPE slope;
    enum WAVE waves[LEDS]; // from left to right, white to blue, i.e. MENU, HOME, BACK, SEARCH, BLUELEFT, BLUERIGHT
};

static struct pattern b(enum CYCLE cycle, enum WAVE wave)
{
    struct pattern pattern = {
        .cycle = cycle,
        .slope = SLOPE_4TH,
    };
    enum KEY_LED led;

    for (led = KEY_LED_MIN; led <= KEY_LED_MAX; ++led) {
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

static enum WAVE special_waves[][LEDS] = {
    {WAVE_143, WAVE_242, WAVE_351, WAVE_44, WAVE_8 | INV, WAVE_8 | INV},
    {WAVE_26 | INV, WAVE_224, WAVE_422, WAVE_62, WAVE_8 | INV, WAVE_8 | INV},
    {WAVE_44, WAVE_44 | INV, WAVE_44 | INV, WAVE_44, WAVE_8 | INV, WAVE_8 | INV},
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

static struct pattern pattern_from_light_state(struct light_state_t const* state)
{
    int flashMS = state->flashOnMS + state->flashOffMS;
    int ratio;
    enum CYCLE cycle = CYCLE_2_10_S;

    ALOGD("liblights: flashMode %d, flashOff %d, flashOn %d\n",
        state->flashMode, state->flashOffMS, state->flashOnMS);

    if (state->flashMode == LIGHT_FLASH_HARDWARE)
        return b(cycle, WAVE_44);

    if (state->flashMode != LIGHT_FLASH_TIMED) // off
        return b(cycle, WAVE_8 | INV);

    if (state->flashOnMS == 0) // off
        return b(cycle, WAVE_8 | INV);

    if (state->flashOffMS == 0) {
        switch (state->flashOnMS) {
        case 1: // always on
            return b(cycle, WAVE_8);
        case 5000:
            cycle = CYCLE_4_19_S;
            break;
        case 2500:
            cycle = CYCLE_2_10_S;
            break;
        case 1000:
            cycle = CYCLE_1_05_S;
            break;
        case 500:
            cycle = CYCLE_0_52_S;
            break;
        case 250:
            cycle = CYCLE_131_MS;
            break;
        }

        return (struct pattern) {
            .special = 1,
            .cycle = cycle,
        };
    }

    cycle = cycle_from_flashMS(flashMS);

    return b(cycle, 7 * state->flashOffMS / flashMS);
}

static int set_light_backlight(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int brightness = rgb_to_brightness(state->color);
    char buf[128];
    int count = snprintf(buf, 128, "%d", brightness);
    (void)dev;

    return write_string(LCD_FILE, buf, count);
}

static int set_light_buttons(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int value[LEDS];
    int brightness = rgb_to_brightness(state->color);
    (void)dev;

    write_int(LED_PATH "button", !!brightness);
    write_int(LED_PATH "touchkey_enabled", !!brightness);

    return 0;
}

#define min(a, b) (((a) < (b)) ? (a) : (b))

static int set_light_notifications(struct light_device_t *dev,
        struct light_state_t const *state)
{
    int value[LEDS];
    struct pattern p = pattern_from_light_state(state);
    enum KEY_LED led;
    int hour;
    char buf[256];
    int o = 0;
    int ret;
    (void)dev;

    if (p.special) {
        /*
         * Use the higher 8 bits of each of the 3 colors to determine
         * the brightness of the white and blue leds.
         * Use the lower 8 bits to determine the special mode.
         */
        int white = rgb_coeff_avg(state->color & 0xf0f0f0, 87, 169, 0) * 0xff / 0xf0;
        int blue = (state->color & 0xf0) * 0xff / 0xf0;

        for (led = KEY_LED_MIN; led <= KEY_LED_MAX; ++led) {
            value[led] = IS_WHITE(led) ? white : blue;
        }

        int rl  = (state->color >> 16) & 0xf;
        int gl = (state->color >> 8) & 0xf;
        int bl = state->color & 0xf;

        p.slope = rl & 0x3;
        size_t wave_idx = gl & 0xf;
        int rtl = bl & 0x1;
        int invert = bl & 0x2 ? INV : 0;

        wave_idx = wave_idx % ARRAY_SIZE(special_waves);

        for (led = KEY_LED_WMIN; led <= KEY_LED_WMAX; ++led) {
            enum KEY_LED led_target = rtl ? KEY_LED_WMSUM - led : led;

            p.waves[led_target] = special_waves[wave_idx][led] ^ invert;
        }

        for (led = KEY_LED_BMIN; led <= KEY_LED_BMAX; ++led) {
            p.waves[led] = special_waves[wave_idx][led];
        }
    } else {
        struct wb wb = rgb_to_wb(state->color);

        for (led = KEY_LED_MIN; led <= KEY_LED_MAX; ++led) {
            value[led] = IS_WHITE(led) ? wb.white : wb.blue;
        }
    }

    ret = snprintf(buf + o, sizeof(buf) - o, "%d %d %d -",
                        p.cycle, p.slope, p.slope);

    if (ret < 0) {
        ALOGE("liblights: Couldn't create pattern string\n");
        return -EINVAL;
    }

    o += min(sizeof(buf) - o, (size_t)ret);

    for (led = KEY_LED_MIN; led <= KEY_LED_MAX; ++led) {
        int inv = p.waves[led] & INV;
        unsigned int wave = p.waves[led] & ~INV;
        unsigned int val = value[led];
        unsigned int val0 = inv & INV ? val : 0;
        unsigned int val1 = !inv ? val : 0;
        ret = snprintf(buf + o, sizeof(buf) - o,
            " %u %u %u,", val0 * 100 / 255, val1 * 100 / 255, wave);

        if (ret < 0) {
            ALOGE("liblights: Couldn't create pattern string\n");
            return -EINVAL;
        }

        o += min(sizeof(buf) - o, (size_t)ret);
    }

    write_int(LED_PATH "brightness", 63);
    write_string(LED_PATH "pattern", buf, o);

    return 0;
}

#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type, member) );})


/** Close the lights device */
static int close_lights(struct hw_device_t *dev)
{
    if (dev) {
        struct light_device_t *ldev =
            container_of(dev, struct light_device_t, common);

        free(ldev);
    }
    return 0;
}

static inline int streq(const char* str1, const char* str2)
{
    return strcmp(str1, str2) == 0;
}

struct hw_module_t HAL_MODULE_INFO_SYM;

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t *module, char const *name,
        struct hw_device_t **device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (module != &HAL_MODULE_INFO_SYM) {
        ALOGE("liblights: module pointer is not as expected\n");
        return -EINVAL;
    }

    ALOGD("liblights: open_lights called for %s\n", name);

    if (streq(LIGHT_ID_BACKLIGHT, name)) {
        set_light = set_light_backlight;
    } else if (streq(LIGHT_ID_BUTTONS, name)) {
        set_light = set_light_buttons;
    } else if (streq(LIGHT_ID_NOTIFICATIONS, name)) {
        set_light = set_light_notifications;
    } else {
        ALOGI("liblights: device %s not supported\n", name);
        return -EINVAL;
    }

    struct light_device_t *dev = calloc(1, sizeof(struct light_device_t));

    if (!dev) {
        ALOGE("liblights: Couldn't alloc light device\n");
        return -ENOMEM;
    }

    dev->common = (struct hw_device_t) {
        .tag = HARDWARE_DEVICE_TAG,
        .module = &HAL_MODULE_INFO_SYM,
        .close = close_lights,
    };
    dev->set_light = set_light;

    *device = &dev->common;

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
