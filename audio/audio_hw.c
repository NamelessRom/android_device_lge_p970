/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2011 Texas Instruments Inc.
 * Copyright (C) 2014 Dheeraj CVR (cvr.dheeraj@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * This module is a derived work from the original contribution of
 * the /device/samsung/tuna/audio/audio_hw.c by Simon Wilson
 *
 */

#define LOG_TAG "audio_hw_primary"
/*#define LOG_NDEBUG 0*/
/*#define LOG_NDEBUG_FUNCTION*/
#ifndef LOG_NDEBUG_FUNCTION
#define LOGFUNC(...) ((void)0)
#else
#define LOGFUNC(...) (ALOGD(__VA_ARGS__))
#endif

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <tinyalsa/asoundlib.h>
#include <audio_utils/resampler.h>
#include <audio_utils/echo_reference.h>
#include <hardware/audio_effect.h>
#include <audio_effects/effect_aec.h>

/* ALSA cards for OMAP */
#define CARD_OMAP_ABE 0
#define CARD_OMAP_USB 2
#define CARD_OMAP_DEFAULT CARD_OMAP_ABE

/* ALSA ports for OMAP */
#define PORT_MM 0
#define PORT_MM2_UL 1
#define PORT_VX 2
#define PORT_TONES 3
#define PORT_VIBRA 4
#define PORT_MODEM 5
#define PORT_MM_LP 6

/* constraint imposed by ABE for CBPr mode: all period sizes must be multiples of 24 */
#define ABE_BASE_FRAME_COUNT 24
/* number of base blocks in a short period (low latency) */
#define SHORT_PERIOD_MULTIPLIER 80  /* 40 ms */
/* number of frames per short period (low latency) */
#define SHORT_PERIOD_SIZE (ABE_BASE_FRAME_COUNT * SHORT_PERIOD_MULTIPLIER)
/* number of short periods in a long period (low power) */
#define LONG_PERIOD_MULTIPLIER 1  /* 40 ms */
/* number of frames per long period (low power) */
#define LONG_PERIOD_SIZE (SHORT_PERIOD_SIZE * LONG_PERIOD_MULTIPLIER)
/* number of periods for playback */
#define PLAYBACK_PERIOD_COUNT 4
/* number of periods for capture */
#define CAPTURE_PERIOD_COUNT 4
/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 5000

#define RESAMPLER_BUFFER_FRAMES (SHORT_PERIOD_SIZE * 2)
#define RESAMPLER_BUFFER_SIZE (4 * RESAMPLER_BUFFER_FRAMES)

#define DEFAULT_OUT_SAMPLING_RATE 48000

/* sampling rate when using MM full power port */
#define MM_FULL_POWER_SAMPLING_RATE 48000
/* sampling rate when using VX port for wide band */
#define VX_WB_SAMPLING_RATE 16000

struct route_setting
{
    char *ctl_name;
    int intval;
    char *strval;
};

/* Audio Routes for LG Optimus Black P970 */
struct route_setting route_standby[] = {
    { .ctl_name = "DAC2 Digital Fine Playback Volume", .intval = 63, },
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 0, },
    { .ctl_name = "PreDriv Playback Volume", .intval = 0, },
    { .ctl_name = "Headset Playback Volume", .intval = 0, },
    { .ctl_name = "Carkit Playback Volume", .intval = 0, },
    { .ctl_name = "Earpiece Playback Volume", .intval = 0, },
    { .ctl_name = "DAC2 AnalogL Playback Volume", .intval = 5, },
    { .ctl_name = "DAC2 AnalogR Playback Volume", .intval = 5, },
    { .ctl_name = "PredriveL Mixer AudioL2", .intval = 0, },
    { .ctl_name = "PredriveR Mixer AudioR2", .intval = 0, },
    { .ctl_name = "Earpiece Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetL Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetR Mixer AudioR2", .intval = 0, },
    { .ctl_name = "Voice", .strval = "OFF", },
    { .ctl_name = "ExtAmp", .strval = "OFF", },
    { .ctl_name = NULL, },
};

struct route_setting route_speaker[] = {
    { .ctl_name = "DAC2 Digital Fine Playback Volume", .intval = 63, },
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 0, },
    { .ctl_name = "PreDriv Playback Volume", .intval = 2, },
    { .ctl_name = "Headset Playback Volume", .intval = 0, },
    { .ctl_name = "Carkit Playback Volume", .intval = 0, },
    { .ctl_name = "Earpiece Playback Volume", .intval = 0, },
    { .ctl_name = "DAC2 AnalogL Playback Volume", .intval = 5, },
    { .ctl_name = "DAC2 AnalogR Playback Volume", .intval = 5, },
    { .ctl_name = "PredriveL Mixer AudioL2", .intval = 1, },
    { .ctl_name = "PredriveR Mixer AudioR2", .intval = 1, },
    { .ctl_name = "Earpiece Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetL Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetR Mixer AudioR2", .intval = 0, },
    { .ctl_name = "Voice", .strval = "OFF", },
    { .ctl_name = "ExtAmp", .strval = "Speaker", },
    { .ctl_name = NULL, },
};

struct route_setting route_speaker_headset[] = {
    { .ctl_name = "DAC2 Digital Fine Playback Volume", .intval = 63, },
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 0, },
    { .ctl_name = "PreDriv Playback Volume", .intval = 2, },
    { .ctl_name = "Headset Playback Volume", .intval = 2, },
    { .ctl_name = "Carkit Playback Volume", .intval = 0, },
    { .ctl_name = "Earpiece Playback Volume", .intval = 0, },
    { .ctl_name = "DAC2 AnalogL Playback Volume", .intval = 5, },
    { .ctl_name = "DAC2 AnalogR Playback Volume", .intval = 5, },
    { .ctl_name = "PredriveL Mixer AudioL2", .intval = 1, },
    { .ctl_name = "PredriveR Mixer AudioR2", .intval = 1, },
    { .ctl_name = "Earpiece Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetL Mixer AudioL2", .intval = 1, },
    { .ctl_name = "HeadsetR Mixer AudioR2", .intval = 1, },
    { .ctl_name = "Voice", .strval = "OFF", },
    { .ctl_name = "ExtAmp", .strval = "Dual", },
    { .ctl_name = NULL, },
};

struct route_setting route_headset[] = {
    { .ctl_name = "DAC2 Digital Fine Playback Volume", .intval = 63, },
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 0, },
    { .ctl_name = "PreDriv Playback Volume", .intval = 0, },
    { .ctl_name = "Headset Playback Volume", .intval = 2, },
    { .ctl_name = "Carkit Playback Volume", .intval = 0, },
    { .ctl_name = "Earpiece Playback Volume", .intval = 0, },
    { .ctl_name = "DAC2 AnalogL Playback Volume", .intval = 6, },
    { .ctl_name = "DAC2 AnalogR Playback Volume", .intval = 6, },
    { .ctl_name = "PredriveL Mixer AudioL2", .intval = 0, },
    { .ctl_name = "PredriveR Mixer AudioR2", .intval = 0, },
    { .ctl_name = "Earpiece Mixer AudioL2", .intval = 0, },
    { .ctl_name = "HeadsetL Mixer AudioL2", .intval = 1, },
    { .ctl_name = "HeadsetR Mixer AudioR2", .intval = 1, },
    { .ctl_name = "Voice", .strval = "OFF", },
    { .ctl_name = "ExtAmp", .strval = "Headset", },
    { .ctl_name = NULL, },
};

struct route_setting route_main_mic[] = {
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .strval = "Off", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .strval = "Main Mic", },
    { .ctl_name = NULL, },
};

struct route_setting route_headset_mic[] = {
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .strval = "Off", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .strval = "Headset Mic", },
    { .ctl_name = "Analog Capture Volume", .intval = 5, },
    { .ctl_name = "TX1 Digital Capture Volume", .intval = 8, },
    { .ctl_name = NULL, },
};

struct route_setting route_input_standby[] = {
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 0, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 0, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 0, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .strval = "Off", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .strval = "Off", },
    { .ctl_name = "Analog Capture Volume", .intval = 0, },
    { .ctl_name = "TX1 Digital Capture Volume", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting route_voicecall_headset[] = {
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 34, },
    { .ctl_name = "Voice", .strval = "Headset", },
    { .ctl_name = "ExtAmp", .strval = "Headsetcall", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 1, },
    { .ctl_name = NULL, },
};

struct route_setting route_voicecall_speaker[] = {
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 34, },
    { .ctl_name = "Voice", .strval = "Speaker", },
    { .ctl_name = "ExtAmp", .strval = "Spkcall", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 1, },
    { .ctl_name = NULL, },
};

struct route_setting route_voicecall_earpiece[] = {
    { .ctl_name = "DAC Voice Digital Downlink Volume", .intval = 37, },
    { .ctl_name = "Voice", .strval = "Receiver", },
    { .ctl_name = "ExtAmp", .strval = "OFF", },
    { .ctl_name = "Analog Left Main Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Right Sub Mic Capture Switch", .intval = 1, },
    { .ctl_name = "Analog Left Headset Mic Capture Switch", .intval = 1, },
    { .ctl_name = NULL, },
};

enum tty_modes {
    TTY_MODE_OFF,
    TTY_MODE_VCO,
    TTY_MODE_HCO,
    TTY_MODE_FULL
};

struct pcm_config pcm_config_mm = {
    .channels = 2,
    .rate = DEFAULT_OUT_SAMPLING_RATE,
    .period_size = LONG_PERIOD_SIZE,
    .period_count = PLAYBACK_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_mm_ul = {
    .channels = 2,
    .rate = MM_FULL_POWER_SAMPLING_RATE,
    .period_size = SHORT_PERIOD_SIZE,
    .period_count = CAPTURE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_vx = {
    .channels = 1,
    .rate = VX_WB_SAMPLING_RATE,
    .period_size = 160,
    .period_count = 2,
    .format = PCM_FORMAT_S16_LE,
};

#define MIN(x, y) ((x) > (y) ? (y) : (x))

struct buffer_remix;

/* buffer_remix: functor for doing in-place buffer manipulations.
 *
 * NB. When remix_func is called, the memory at `buf` must be at least
 * as large as frames * sample_size * MAX(in_chans, out_chans).
 */
struct buffer_remix {
    void (*remix_func)(struct buffer_remix *data, void *buf, size_t frames);
    size_t sample_size; /* size of one audio sample, in bytes */
    size_t in_chans;    /* number of input channels */
    size_t out_chans;   /* number of output channels */
};

struct omap_audio_device {
    struct audio_hw_device hw_device;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct mixer *mixer;
    int mode;
    int out_device;
    int in_device;
    int in_call;
    struct omap_stream_in *active_input;
    struct omap_stream_out *active_output;
    bool mic_mute;
    int tty_mode;
    struct echo_reference_itfe *echo_reference;
    int input_requires_stereo;
    bool bluetooth_nrec;
};

struct omap_stream_out {
    struct audio_stream_out stream;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct pcm_config config;
    struct pcm *pcm;
    struct resampler_itfe *resampler;
    char *buffer;
    int standby;
    struct echo_reference_itfe *echo_reference;
    int write_threshold;

    struct omap_audio_device *dev;
};

#define MAX_PREPROCESSORS 3 /* maximum one AGC + one NS + one AEC per input stream */

struct omap_stream_in {
    struct audio_stream_in stream;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct pcm_config config;
    struct pcm *pcm;
    int device;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t frames_in;
    unsigned int requested_rate;
    int standby;
    int source;
    struct echo_reference_itfe *echo_reference;
    bool need_echo_reference;
    effect_handle_t preprocessors[MAX_PREPROCESSORS];
    int num_preprocessors;
    int16_t *proc_buf;
    size_t proc_buf_size;
    size_t proc_frames_in;
    int16_t *ref_buf;
    size_t ref_buf_size;
    size_t ref_frames_in;
    int read_status;
    struct buffer_remix *remix_at_driver; /* adapt hw chan count to client */

    struct omap_audio_device *dev;
};

/**
 * NOTE: when multiple mutexes have to be acquired, always respect the following order:
 *        hw device > in stream > out stream
 */


static void select_output_device(struct omap_audio_device *adev);
static void select_input_device(struct omap_audio_device *adev);
static int adev_set_voice_volume(struct audio_hw_device *dev, float volume);
static int do_input_standby(struct omap_stream_in *in);
static int do_output_standby(struct omap_stream_out *out);

/* Implementation of buffer_remix::remix_func that removes
 * channels in place without doing any other processing.  The
 * extra channels are truncated.
 */
static void remove_channels_from_buf(struct buffer_remix *data, void *buf, size_t frames)
{
    size_t samp_size, in_frame, out_frame;
    size_t N, c;
    char *s, *d;

    LOGFUNC("%s(%p, %p, %d)", __FUNCTION__, data, buf, frames);
    if (frames == 0)
        return;


    samp_size = data->sample_size;
    in_frame = data->in_chans * samp_size;
    out_frame = data->out_chans * samp_size;

    if (out_frame >= in_frame) {
        ALOGE("BUG: remove_channels_from_buf() can not add channels to a buffer.\n");
        return;
    }

    N = frames - 1;
    d = (char*)buf + out_frame;
    s = (char*)buf + in_frame;

    /* take the first several channels and
     * truncate the rest
     */
    while (N--) {
        for (c=0 ; c < out_frame ; ++c)
            d[c] = s[c];
        d += out_frame;
        s += in_frame;
    }
}

static void setup_stereo_to_mono_input_remix(struct omap_stream_in *in)
{
    struct buffer_remix *br = (struct buffer_remix *)malloc(sizeof(struct buffer_remix));

    LOGFUNC("%s(%p)", __FUNCTION__, in);


    if (br) {
        br->remix_func = remove_channels_from_buf;
        br->sample_size = audio_stream_frame_size(&in->stream.common) / in->config.channels;
        br->in_chans = 2;
        br->out_chans = 1;
    } else
        ALOGE("Could not allocate memory for struct buffer_remix\n");

    if (in->buffer) {
        size_t chans = (br->in_chans > br->out_chans) ? br->in_chans : br->out_chans;
        free(in->buffer);
        in->buffer = malloc(in->config.period_size * br->sample_size * chans);
        if (!in->buffer)
            ALOGE("Could not reallocate memory for input buffer\n");
    }

    if (in->remix_at_driver)
        free(in->remix_at_driver);
    in->remix_at_driver = br;
}

/* The enable flag when 0 makes the assumption that enums are disabled by
 * "Off" and integers/booleans by 0 */

static int set_route_by_array(struct mixer *mixer, struct route_setting *route)
{
    struct mixer_ctl *ctl;
    unsigned int i, j;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, mixer, route);

    /* Go through the route array and set each value */
    i = 0;
    while (route[i].ctl_name) {
        ctl = mixer_get_ctl_by_name(mixer, route[i].ctl_name);
        if (!ctl)
            return -EINVAL;

        if (route[i].strval) {
            mixer_ctl_set_enum_by_string(ctl, route[i].strval);
        } else {
            /* This ensures multiple (i.e. stereo) values are set jointly */
            for (j = 0; j < mixer_ctl_get_num_values(ctl); j++) {
                mixer_ctl_set_value(ctl, j, route[i].intval);
            }
        }
        i++;
    }

    return 0;
}

static void set_incall_device(struct omap_audio_device *adev)
{
    if (adev->out_device & AUDIO_DEVICE_OUT_EARPIECE) {
        ALOGD("%s: AUDIO_DEVICE_OUT_EARPIECE", __func__);
        set_route_by_array(adev->mixer, route_voicecall_earpiece);
    } else if (adev->out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
        ALOGD("%s: AUDIO_DEVICE_OUT_WIRED_HEADSET", __func__);
        set_route_by_array(adev->mixer, route_voicecall_headset);
    } else if (adev->out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        ALOGD("%s: AUDIO_DEVICE_OUT_WIRED_HEADPHONE", __func__);
        set_route_by_array(adev->mixer, route_voicecall_headset);
    } else if (adev->out_device & AUDIO_DEVICE_OUT_SPEAKER) {
        ALOGD("%s: AUDIO_DEVICE_OUT_SPEAKER", __func__);
        set_route_by_array(adev->mixer, route_voicecall_speaker);
    }
    return;
}

static void force_all_standby(struct omap_audio_device *adev)
{
    struct omap_stream_in *in;
    struct omap_stream_out *out;

    LOGFUNC("%s(%p)", __FUNCTION__, adev);

    if (adev->active_output) {
        out = adev->active_output;
        pthread_mutex_lock(&out->lock);
        do_output_standby(out);
        pthread_mutex_unlock(&out->lock);
    }
    if (adev->active_input) {
        in = adev->active_input;
        pthread_mutex_lock(&in->lock);
        do_input_standby(in);
        pthread_mutex_unlock(&in->lock);
    }
}

static void select_mode(struct omap_audio_device *adev)
{
    LOGFUNC("%s(%p)", __FUNCTION__, adev);

    if (adev->mode == AUDIO_MODE_IN_CALL) {
        ALOGE("Entering IN_CALL state, in_call=%d", adev->in_call);
        if (!adev->in_call) {
            force_all_standby(adev);
            /* force earpiece route for in call state if speaker is the
            only currently selected route. This prevents having to tear
            down the modem PCMs to change route from speaker to earpiece
            after the ringtone is played, but doesn't cause a route
            change if a headset or bt device is already connected. If
            speaker is not the only thing active, just remove it from
            the route. We'll assume it'll never be used initally during
            a call. This works because we're sure that the audio policy
            manager will update the output device after the audio mode
            change, even if the device selection did not change. */
            if (adev->out_device == AUDIO_DEVICE_OUT_SPEAKER) {
                adev->out_device = AUDIO_DEVICE_OUT_EARPIECE;
                adev->in_device = AUDIO_DEVICE_IN_BUILTIN_MIC & ~AUDIO_DEVICE_BIT_IN;
            } else
                adev->out_device &= ~AUDIO_DEVICE_OUT_SPEAKER;
            select_output_device(adev);
            adev->in_call = 1;
        }
    } else {
        ALOGE("Leaving IN_CALL state, in_call=%d, mode=%d",
             adev->in_call, adev->mode);
        if (adev->in_call) {
            adev->in_call = 0;
            force_all_standby(adev);
            select_output_device(adev);
            select_input_device(adev);
        }
    }
}

static void select_output_device(struct omap_audio_device *adev)
{
    int headset_on;
    int headphone_on;
    int speaker_on;
    int earpiece_on;
    int bt_on;

    LOGFUNC("%s(%p)", __FUNCTION__, adev);

    {
        headset_on = adev->out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET;
        headphone_on = adev->out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
        speaker_on = adev->out_device & AUDIO_DEVICE_OUT_SPEAKER;
        earpiece_on = adev->out_device & AUDIO_DEVICE_OUT_EARPIECE;
        bt_on = adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO;

        /* force rx path according to TTY mode when in call */
        if (adev->mode == AUDIO_MODE_IN_CALL && !bt_on) {
            switch(adev->tty_mode) {
                case TTY_MODE_FULL:
                case TTY_MODE_VCO:
                    /* rx path to headphones */
                    headphone_on = 1;
                    headset_on = 0;
                    speaker_on = 0;
                    earpiece_on = 0;
                    break;
                case TTY_MODE_HCO:
                    /* rx path to device speaker */
                    headphone_on = 0;
                    headset_on = 0;
                    speaker_on = 1;
                    earpiece_on = 0;
                    break;
                case TTY_MODE_OFF:
                default:
                    break;
            }
        }

        if (speaker_on && headset_on) {
            ALOGE("%s: set route: route_speaker_headset", __func__);
            set_route_by_array(adev->mixer, route_speaker_headset);
        } else if (speaker_on) {
            ALOGE("%s: set route: route_speaker", __func__);
            set_route_by_array(adev->mixer, route_speaker);
        } else if (headset_on || headphone_on) {
            ALOGE("%s: set route: route_headset", __func__);
            set_route_by_array(adev->mixer, route_headset);
        }

        if (adev->mode == AUDIO_MODE_IN_CALL)
            set_incall_device(adev);
    }
}

static void select_input_device(struct omap_audio_device *adev)
{
    int input_device = AUDIO_DEVICE_BIT_IN | adev->in_device;

    switch(input_device) {
        case AUDIO_DEVICE_IN_BUILTIN_MIC:
        case AUDIO_DEVICE_IN_BACK_MIC:
            ALOGD("%s: AUDIO_DEVICE_IN_MIC", __func__);
            set_route_by_array(adev->mixer, route_main_mic);
            break;
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET:
            ALOGD("%s: AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET", __func__);
            break;
        case AUDIO_DEVICE_IN_WIRED_HEADSET:
            ALOGD("%s: AUDIO_DEVICE_IN_WIRED_HEADSET", __func__);
            set_route_by_array(adev->mixer, route_headset_mic);
            break;
        default:
            break;
    }
}

/* must be called with hw device and output stream mutexes locked */
static int start_output_stream(struct omap_stream_out *out)
{
    struct omap_audio_device *adev = out->dev;
    unsigned int card = CARD_OMAP_DEFAULT;
    unsigned int port = PORT_MM;

    LOGFUNC("%s(%p)", __FUNCTION__, adev);

    adev->active_output = out;
    if (adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO)
        out->config.rate = MM_FULL_POWER_SAMPLING_RATE;
    else
        out->config.rate = DEFAULT_OUT_SAMPLING_RATE;

    if (adev->mode != AUDIO_MODE_IN_CALL) {
        /* FIXME: only works if only one output can be active at a time */
        select_output_device(adev);
    }

    if((adev->out_device & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET) ||
        (adev->out_device & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)) {
        card = CARD_OMAP_USB;
        port = PORT_MM;
    }
    /* default to low power:
     *  NOTE: PCM_NOIRQ mode is required to dynamically scale avail_min
     */
    out->write_threshold = PLAYBACK_PERIOD_COUNT * LONG_PERIOD_SIZE;
    out->config.start_threshold = SHORT_PERIOD_SIZE * 2;
    out->config.avail_min = LONG_PERIOD_SIZE,

    out->pcm = pcm_open(card, port, PCM_OUT | PCM_MMAP, &out->config);

    if (!pcm_is_ready(out->pcm)) {
        ALOGE("cannot open pcm_out driver: %s", pcm_get_error(out->pcm));
        pcm_close(out->pcm);
        adev->active_output = NULL;
        return -ENOMEM;
    }

    if (adev->echo_reference != NULL)
        out->echo_reference = adev->echo_reference;
    if (out->resampler)
        out->resampler->reset(out->resampler);

    return 0;
}

static int check_input_parameters(uint32_t sample_rate, int format, int channel_count)
{
    LOGFUNC("%s(%d, %d, %d)", __FUNCTION__, sample_rate, format, channel_count);

    if (format != AUDIO_FORMAT_PCM_16_BIT) {
        return -EINVAL;
    }

    if ((channel_count < 1) || (channel_count > 2)) {
        return -EINVAL;
    }

    switch(sample_rate) {
    case 8000:
    case 11025:
    case 16000:
    case 22050:
    case 24000:
    case 32000:
    case 44100:
    case 48000:
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static size_t get_input_buffer_size(uint32_t sample_rate, int format, int channel_count)
{
    size_t size;
    size_t device_rate;

    LOGFUNC("%s(%d, %d, %d)", __FUNCTION__, sample_rate, format, channel_count);

    /* take resampling into account and return the closest majoring
    multiple of 16 frames, as audioflinger expects audio buffers to
    be a multiple of 16 frames */
    size = (pcm_config_mm_ul.period_size * sample_rate) / pcm_config_mm_ul.rate;
    size = ((size + 15) / 16) * 16;

    return size * channel_count * sizeof(short);
}

static void add_echo_reference(struct omap_stream_out *out,
                               struct echo_reference_itfe *reference)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, out, reference);

    pthread_mutex_lock(&out->lock);
    out->echo_reference = reference;
    pthread_mutex_unlock(&out->lock);
}

static void remove_echo_reference(struct omap_stream_out *out,
                                  struct echo_reference_itfe *reference)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, out, reference);

    pthread_mutex_lock(&out->lock);
    if (out->echo_reference == reference) {
        /* stop writing to echo reference */
        reference->write(reference, NULL);
        out->echo_reference = NULL;
    }
    pthread_mutex_unlock(&out->lock);
}

static void put_echo_reference(struct omap_audio_device *adev,
                          struct echo_reference_itfe *reference)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, adev, reference);

    if (adev->echo_reference != NULL &&
            reference == adev->echo_reference) {
        if (adev->active_output != NULL)
            remove_echo_reference(adev->active_output, reference);
        release_echo_reference(reference);
        adev->echo_reference = NULL;
    }
}

static struct echo_reference_itfe *get_echo_reference(struct omap_audio_device *adev,
                                               audio_format_t format,
                                               uint32_t channel_count,
                                               uint32_t sampling_rate)
{
    LOGFUNC("%s(%p, 0x%08x, 0x%04x, %d)", __FUNCTION__, adev, format,
                                                channel_count, sampling_rate);

    put_echo_reference(adev, adev->echo_reference);
    if (adev->active_output != NULL) {
        struct audio_stream *stream = &adev->active_output->stream.common;
        uint32_t wr_channel_count = popcount(stream->get_channels(stream));
        uint32_t wr_sampling_rate = stream->get_sample_rate(stream);

        int status = create_echo_reference(AUDIO_FORMAT_PCM_16_BIT,
                                           channel_count,
                                           sampling_rate,
                                           AUDIO_FORMAT_PCM_16_BIT,
                                           wr_channel_count,
                                           wr_sampling_rate,
                                           &adev->echo_reference);
        if (status == 0)
            add_echo_reference(adev->active_output, adev->echo_reference);
    }
    return adev->echo_reference;
}

static int get_playback_delay(struct omap_stream_out *out,
                       size_t frames,
                       struct echo_reference_buffer *buffer)
{
    size_t kernel_frames;
    int status;

    LOGFUNC("%s(%p, %ul, %p)", __FUNCTION__, out, frames, buffer);

    status = pcm_get_htimestamp(out->pcm, &kernel_frames, &buffer->time_stamp);
    if (status < 0) {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGV("get_playback_delay(): pcm_get_htimestamp error,"
                "setting playbackTimestamp to 0");
        return status;
    }

    kernel_frames = pcm_get_buffer_size(out->pcm) - kernel_frames;

    /* adjust render time stamp with delay added by current driver buffer.
     * Add the duration of current frame as we want the render time of the last
     * sample being written. */
    buffer->delay_ns = (long)(((int64_t)(kernel_frames + frames)* 1000000000)/
                            MM_FULL_POWER_SAMPLING_RATE);

    return 0;
}

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return DEFAULT_OUT_SAMPLING_RATE;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, stream, rate);

    return 0;
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
    struct omap_stream_out *out = (struct omap_stream_out *)stream;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    /* take resampling into account and return the closest majoring
    multiple of 16 frames, as audioflinger expects audio buffers to
    be a multiple of 16 frames */
    size_t size = (SHORT_PERIOD_SIZE * DEFAULT_OUT_SAMPLING_RATE) / out->config.rate;
    size = ((size + 15) / 16) * 16;
    return size * audio_stream_frame_size((struct audio_stream *)stream);
}

static audio_channel_mask_t out_get_channels(const struct audio_stream *stream)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return AUDIO_CHANNEL_OUT_STEREO;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return AUDIO_FORMAT_PCM_16_BIT;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return 0;
}

/* must be called with hw device and output stream mutexes locked */
static int do_output_standby(struct omap_stream_out *out)
{
    struct omap_audio_device *adev = out->dev;

    LOGFUNC("%s(%p)", __FUNCTION__, out);

    if (!out->standby) {
        pcm_close(out->pcm);
        out->pcm = NULL;

        if (adev->mode != AUDIO_MODE_IN_CALL && !adev->active_input)
            set_route_by_array(adev->mixer, route_standby);

        adev->active_output = 0;

        /* stop writing to echo reference */
        if (out->echo_reference != NULL) {
            out->echo_reference->write(out->echo_reference, NULL);
            out->echo_reference = NULL;
        }
        out->standby = 1;
    }

    return 0;
}

static int out_standby(struct audio_stream *stream)
{
    struct omap_stream_out *out = (struct omap_stream_out *)stream;
    int status;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    pthread_mutex_lock(&out->dev->lock);
    pthread_mutex_lock(&out->lock);
    status = do_output_standby(out);
    pthread_mutex_unlock(&out->lock);
    pthread_mutex_unlock(&out->dev->lock);
    return status;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, stream, fd);

    return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct omap_stream_out *out = (struct omap_stream_out *)stream;
    struct omap_audio_device *adev = out->dev;
    struct omap_stream_in *in;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret, val = 0;
    bool force_input_standby = false;

    ALOGE("%s(%p, %s)", __FUNCTION__, stream, kvpairs);

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        pthread_mutex_lock(&adev->lock);
        pthread_mutex_lock(&out->lock);
        if ((adev->out_device != val) && (val != 0)) {
            if (out == adev->active_output) {
                do_output_standby(out);
                /* a change in output device may change the microphone selection */
                if (adev->active_input &&
                        adev->active_input->source == AUDIO_SOURCE_VOICE_COMMUNICATION) {
                    force_input_standby = true;
                }
            }
            adev->out_device = val;
            select_output_device(adev);
        }

        pthread_mutex_unlock(&out->lock);
        if (force_input_standby) {
            in = adev->active_input;
            pthread_mutex_lock(&in->lock);
            do_input_standby(in);
            pthread_mutex_unlock(&in->lock);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    str_parms_destroy(parms);
    return ret;
}

static char * out_get_parameters(const struct audio_stream *stream, const char *keys)
{
    LOGFUNC("%s(%p, %s)", __FUNCTION__, stream, keys);

    return strdup("");
}

static uint32_t out_get_latency(const struct audio_stream_out *stream)
{
    struct omap_stream_out *out = (struct omap_stream_out *)stream;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);
    return (SHORT_PERIOD_SIZE * PLAYBACK_PERIOD_COUNT * 1000) / out->config.rate;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
                          float right)
{
    LOGFUNC("%s(%p, %f, %f)", __FUNCTION__, stream, left, right);

    return -ENOSYS;
}

static ssize_t out_write(struct audio_stream_out *stream, const void* buffer,
                         size_t bytes)
{
    int ret;
    struct omap_stream_out *out = (struct omap_stream_out *)stream;
    struct omap_audio_device *adev = out->dev;
    size_t frame_size = audio_stream_frame_size(&out->stream.common);
    size_t in_frames = bytes / frame_size;
    size_t out_frames = RESAMPLER_BUFFER_SIZE / frame_size;
    bool force_input_standby = false;
    struct omap_stream_in *in;
    int kernel_frames;
    void *buf = (void *)0xDEADBAAD;

    LOGFUNC("%s(%p, %p, %d)", __FUNCTION__, stream, buffer, bytes);

    /* acquiring hw device mutex systematically is useful if a low priority thread is waiting
     * on the output stream mutex - e.g. executing select_mode() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);
    if (out->standby) {
        ret = start_output_stream(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            goto exit;
        }
        out->standby = 0;
        /* a change in output device may change the microphone selection */
        if (adev->active_input &&
                adev->active_input->source == AUDIO_SOURCE_VOICE_COMMUNICATION)
            force_input_standby = true;
    }
    pthread_mutex_unlock(&adev->lock);

    /* only use resampler if required */
    if (out->config.rate != DEFAULT_OUT_SAMPLING_RATE) {
        if (!out->resampler) {
            ret = create_resampler(DEFAULT_OUT_SAMPLING_RATE,
                    MM_FULL_POWER_SAMPLING_RATE,
                    2,
                    RESAMPLER_QUALITY_DEFAULT,
                    NULL,
                    &out->resampler);
            if (ret != 0)
                goto exit;
            out->buffer = malloc(RESAMPLER_BUFFER_SIZE); /* todo: allow for reallocing */
        }
        out->resampler->resample_from_input(out->resampler,
                (int16_t *)buffer,
                &in_frames,
                (int16_t *)out->buffer,
                &out_frames);
        buf = out->buffer;
    } else {
        out_frames = in_frames;
        buf = (void *)buffer;
    }
    if (out->echo_reference != NULL) {
        struct echo_reference_buffer b;
        b.raw = (void *)buffer;
        b.frame_count = in_frames;

        get_playback_delay(out, out_frames, &b);
        out->echo_reference->write(out->echo_reference, &b);
    }

    /* do not allow more than out->write_threshold frames in kernel pcm driver buffer */
    do {
        struct timespec time_stamp;

        if (pcm_get_htimestamp(out->pcm, (unsigned int *)&kernel_frames, &time_stamp) < 0)
            break;
        kernel_frames = pcm_get_buffer_size(out->pcm) - kernel_frames;
        if (kernel_frames > out->write_threshold) {
            unsigned long time = (unsigned long)
                    (((int64_t)(kernel_frames - out->write_threshold) * 1000000) /
                            MM_FULL_POWER_SAMPLING_RATE);
            if (time < MIN_WRITE_SLEEP_US)
                time = MIN_WRITE_SLEEP_US;
            usleep(time);
        }
    } while (kernel_frames > out->write_threshold);

    ret = pcm_mmap_write(out->pcm, (void *)buf, out_frames * frame_size);

exit:
    pthread_mutex_unlock(&out->lock);

    if (ret != 0) {
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               out_get_sample_rate(&stream->common));
    }

    if (force_input_standby) {
        pthread_mutex_lock(&adev->lock);
        if (adev->active_input) {
            in = adev->active_input;
            pthread_mutex_lock(&in->lock);
            do_input_standby(in);
            pthread_mutex_unlock(&in->lock);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, stream, dsp_frames);

    return -EINVAL;
}

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, stream, effect);

    return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    LOGFUNC("%s(%p, %p)", __FUNCTION__, stream, effect);

    return 0;
}

/** audio_stream_in implementation **/

/* must be called with hw device and input stream mutexes locked */
static int start_input_stream(struct omap_stream_in *in)
{
    int ret = 0;
    unsigned int card = CARD_OMAP_DEFAULT;
    unsigned int device = PORT_MM;
    struct omap_audio_device *adev = in->dev;

    LOGFUNC("%s(%p)", __FUNCTION__, in);

    adev->active_input = in;

    if (adev->mode != AUDIO_MODE_IN_CALL) {
        adev->in_device = in->device;
        select_input_device(adev);
    }

    if (adev->input_requires_stereo && (in->config.channels == 1))
        setup_stereo_to_mono_input_remix(in);

    if (in->need_echo_reference && in->echo_reference == NULL)
        in->echo_reference = get_echo_reference(adev,
                                        AUDIO_FORMAT_PCM_16_BIT,
                                        in->config.channels,
                                        in->requested_rate);

    /* this assumes routing is done previously */
    if (in->remix_at_driver)
        in->config.channels = in->remix_at_driver->in_chans;

    in->pcm = pcm_open(card, device, PCM_IN, &in->config);
    if (in->remix_at_driver)
        in->config.channels = in->remix_at_driver->out_chans;
    if (!pcm_is_ready(in->pcm)) {
        ALOGE("cannot open pcm_in driver: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        adev->active_input = NULL;
        return -ENOMEM;
    }

    /* if no supported sample rate is available, use the resampler */
    if (in->resampler) {
        in->resampler->reset(in->resampler);
        in->frames_in = 0;
    }
    return 0;
}

static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, stream, rate);

    return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return get_input_buffer_size(in->requested_rate,
                                 AUDIO_FORMAT_PCM_16_BIT,
                                 in->config.channels);
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    if (in->config.channels == 1) {
        return AUDIO_CHANNEL_IN_MONO;
    } else {
        return AUDIO_CHANNEL_IN_STEREO;
    }
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, stream, format);

    return 0;
}

/* must be called with hw device and input stream mutexes locked */
static int do_input_standby(struct omap_stream_in *in)
{
    struct omap_audio_device *adev = in->dev;

    LOGFUNC("%s(%p)", __FUNCTION__, in);

    if (!in->standby) {
        pcm_close(in->pcm);
        in->pcm = NULL;

        adev->active_input = 0;
        if (adev->mode != AUDIO_MODE_IN_CALL) {
            adev->in_device = AUDIO_DEVICE_NONE;
            select_input_device(adev);
            set_route_by_array(adev->mixer, route_input_standby);
        }

        if (in->echo_reference != NULL) {
            /* stop reading from echo reference */
            in->echo_reference->read(in->echo_reference, NULL);
            put_echo_reference(adev, in->echo_reference);
            in->echo_reference = NULL;
        }
        in->standby = 1;
    }
    return 0;
}

static int in_standby(struct audio_stream *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    int status;

    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    status = do_input_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);
    return status;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, stream, fd);

    return 0;
}
static int in_fm_routing(struct audio_stream *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    int ret;

   LOGFUNC("%s(%p)", __FUNCTION__, stream);

    if (in->standby) {
        ret = start_input_stream(in);
        if (ret == 0)
            in->standby = 0;
    }
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    struct omap_audio_device *adev = in->dev;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret, val = 0;
    bool do_standby = false;

    LOGFUNC("%s(%p, %s)", __FUNCTION__, stream, kvpairs);

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_INPUT_SOURCE, value, sizeof(value));

    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    if (ret >= 0) {
        val = atoi(value);
        /* no audio source uses val == 0 */
        if ((in->source != val) && (val != 0)) {
            in->source = val;
            do_standby = true;
        }
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value) & ~AUDIO_DEVICE_BIT_IN;
        if ((in->device != val) && (val != 0)) {
            in->device = val;
            do_standby = true;
        }
    }

    if (do_standby)
        do_input_standby(in);

    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&adev->lock);

    str_parms_destroy(parms);
    return ret;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    LOGFUNC("%s(%p, %s)", __FUNCTION__, stream, keys);

    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    LOGFUNC("%s(%p, %f)", __FUNCTION__, stream, gain);

    return 0;
}

static void get_capture_delay(struct omap_stream_in *in,
                       size_t frames,
                       struct echo_reference_buffer *buffer)
{

    /* read frames available in kernel driver buffer */
    size_t kernel_frames;
    struct timespec tstamp;
    long buf_delay;
    long rsmp_delay;
    long kernel_delay;
    long delay_ns;

    LOGFUNC("%s(%p, %ul, %p)", __FUNCTION__, in, frames, buffer);

    if (pcm_get_htimestamp(in->pcm, &kernel_frames, &tstamp) < 0) {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGW("read get_capture_delay(): pcm_htimestamp error");
        return;
    }

    /* read frames available in audio HAL input buffer
     * add number of frames being read as we want the capture time of first sample
     * in current buffer */
    buf_delay = (long)(((int64_t)(in->frames_in + in->proc_frames_in) * 1000000000)
                                    / in->config.rate);
    /* add delay introduced by resampler */
    rsmp_delay = 0;
    if (in->resampler) {
        rsmp_delay = in->resampler->delay_ns(in->resampler);
    }

    kernel_delay = (long)(((int64_t)kernel_frames * 1000000000) / in->config.rate);

    delay_ns = kernel_delay + buf_delay + rsmp_delay;

    buffer->time_stamp = tstamp;
    buffer->delay_ns   = delay_ns;
    ALOGV("get_capture_delay time_stamp = [%ld].[%ld], delay_ns: [%d],"
         " kernel_delay:[%ld], buf_delay:[%ld], rsmp_delay:[%ld], kernel_frames:[%d], "
         "in->frames_in:[%d], in->proc_frames_in:[%d], frames:[%d]",
         buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns,
         kernel_delay, buf_delay, rsmp_delay, kernel_frames,
         in->frames_in, in->proc_frames_in, frames);

}

static int32_t update_echo_reference(struct omap_stream_in *in, size_t frames)
{
    struct echo_reference_buffer b;
    b.delay_ns = 0;

    LOGFUNC("%s(%p, %ul)", __FUNCTION__, in, frames);

    ALOGV("update_echo_reference, frames = [%d], in->ref_frames_in = [%d],  "
          "b.frame_count = [%d]",
         frames, in->ref_frames_in, frames - in->ref_frames_in);
    if (in->ref_frames_in < frames) {
        if (in->ref_buf_size < frames) {
            in->ref_buf_size = frames;
            in->ref_buf = (int16_t *)realloc(in->ref_buf,
                                             in->ref_buf_size *
                                                 in->config.channels * sizeof(int16_t));
        }

        b.frame_count = frames - in->ref_frames_in;
        b.raw = (void *)(in->ref_buf + in->ref_frames_in * in->config.channels);

        get_capture_delay(in, frames, &b);

        if (in->echo_reference->read(in->echo_reference, &b) == 0)
        {
            in->ref_frames_in += b.frame_count;
            ALOGV("update_echo_reference: in->ref_frames_in:[%d], "
                    "in->ref_buf_size:[%d], frames:[%d], b.frame_count:[%d]",
                 in->ref_frames_in, in->ref_buf_size, frames, b.frame_count);
        }
    } else
        ALOGW("update_echo_reference: NOT enough frames to read ref buffer");
    return b.delay_ns;
}

static int set_preprocessor_param(effect_handle_t handle,
                           effect_param_t *param)
{
    uint32_t size = sizeof(int);
    uint32_t psize = ((param->psize - 1) / sizeof(int) + 1) * sizeof(int) +
                        param->vsize;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, handle, param);

    int status = (*handle)->command(handle,
                                   EFFECT_CMD_SET_PARAM,
                                   sizeof (effect_param_t) + psize,
                                   param,
                                   &size,
                                   &param->status);
    if (status == 0)
        status = param->status;

    return status;
}

static int set_preprocessor_echo_delay(effect_handle_t handle,
                                     int32_t delay_us)
{
    uint32_t buf[sizeof(effect_param_t) / sizeof(uint32_t) + 2];
    effect_param_t *param = (effect_param_t *)buf;

    LOGFUNC("%s(%p, %d)", __FUNCTION__, handle, delay_us);

    param->psize = sizeof(uint32_t);
    param->vsize = sizeof(uint32_t);
    *(uint32_t *)param->data = AEC_PARAM_ECHO_DELAY;
    *((int32_t *)param->data + 1) = delay_us;

    return set_preprocessor_param(handle, param);
}

static int push_echo_reference(struct omap_stream_in *in, size_t frames)
{
    /* read frames from echo reference buffer and update echo delay
     * in->ref_frames_in is updated with frames available in in->ref_buf */
    int32_t delay_us = update_echo_reference(in, frames)/1000;
    int i;
    audio_buffer_t buf;

    LOGFUNC("%s(%p, %ul)", __FUNCTION__, in, frames);

    if (in->ref_frames_in < frames)
        frames = in->ref_frames_in;

    buf.frameCount = frames;
    buf.raw = in->ref_buf;

    for (i = 0; i < in->num_preprocessors; i++) {
        if ((*in->preprocessors[i])->process_reverse == NULL)
            continue;

        (*in->preprocessors[i])->process_reverse(in->preprocessors[i],
                                               &buf,
                                               NULL);
        set_preprocessor_echo_delay(in->preprocessors[i], delay_us);
    }

    in->ref_frames_in -= buf.frameCount;
    if (in->ref_frames_in) {
        memcpy(in->ref_buf,
               in->ref_buf + buf.frameCount * in->config.channels,
               in->ref_frames_in * in->config.channels * sizeof(int16_t));
    }
    // return the number of frames written to the preprocessor in this call
    return buf.frameCount;
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer)
{
    struct omap_stream_in *in;
    struct buffer_remix *remix;
    size_t hw_frame_size;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, buffer_provider, buffer);

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct omap_stream_in *)((char *)buffer_provider -
                                   offsetof(struct omap_stream_in, buf_provider));
    remix = in->remix_at_driver;

    if (in->pcm == NULL) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
        return -ENODEV;
    }

    if (remix)
        hw_frame_size = remix->in_chans * remix->sample_size;
    else
        hw_frame_size = audio_stream_frame_size(&in->stream.common);

    if (in->frames_in == 0) {
        in->read_status = pcm_read(in->pcm,
                                   (void*)in->buffer,
                                   in->config.period_size * hw_frame_size);
        if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d", in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->frames_in = in->config.period_size;

        if (remix)
            remix->remix_func(remix, in->buffer, in->frames_in);
    }

    buffer->frame_count = (buffer->frame_count > in->frames_in) ?
                                in->frames_in : buffer->frame_count;
    buffer->i16 = in->buffer + (in->config.period_size - in->frames_in) *
                                                in->config.channels;

    return in->read_status;

}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer)
{
    struct omap_stream_in *in;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, buffer_provider, buffer);

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct omap_stream_in *)((char *)buffer_provider -
                                   offsetof(struct omap_stream_in, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified */
static ssize_t read_frames(struct omap_stream_in *in, void *buffer, ssize_t frames)
{
    ssize_t frames_wr = 0;
    size_t frame_size;

    LOGFUNC("%s(%p, %p, %ld)", __FUNCTION__, in, buffer, frames);

    if (in->remix_at_driver)
        frame_size = in->remix_at_driver->out_chans * in->remix_at_driver->sample_size;
    else
        frame_size = audio_stream_frame_size(&in->stream.common);

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in->resampler != NULL) {
            in->resampler->resample_from_provider(in->resampler,
                    (int16_t *)((char *)buffer + frames_wr * frame_size),
                    &frames_rd);
        } else {
            struct resampler_buffer buf = {
                    { raw : NULL, },
                    frame_count : frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer +
                        frames_wr * frame_size,
                        buf.raw,
                        buf.frame_count * frame_size);
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }
    return frames_wr;
}

/* process_frames() reads frames from kernel driver (via read_frames()),
 * calls the active audio pre processings and output the number of frames requested
 * to the buffer specified */
static ssize_t process_frames(struct omap_stream_in *in, void* buffer, ssize_t frames)
{
    ssize_t frames_wr = 0;
    audio_buffer_t in_buf;
    audio_buffer_t out_buf;
    int i;
    ssize_t prepro_wr_cnt;

    LOGFUNC("%s(%p, %p, %ld)", __FUNCTION__, in, buffer, frames);

    while (frames_wr < frames) {
        /* first reload enough frames at the end of process input buffer */
        if (in->proc_frames_in < (size_t)frames) {
            ssize_t frames_rd;

            if (in->proc_buf_size < (size_t)frames) {
                in->proc_buf_size = (size_t)frames;
                in->proc_buf = (int16_t *)realloc(in->proc_buf,
                                         in->proc_buf_size *
                                             in->config.channels * sizeof(int16_t));
                ALOGV("process_frames(): in->proc_buf %p size extended to %d frames",
                     in->proc_buf, in->proc_buf_size);
            }
            frames_rd = read_frames(in,
                                    in->proc_buf +
                                        in->proc_frames_in * in->config.channels,
                                    frames - in->proc_frames_in);
            if (frames_rd < 0) {
                frames_wr = frames_rd;
                break;
            }
            in->proc_frames_in += frames_rd;
        }

        /* make sure that the number of mic signal frames written to the
         * preprocessor is the same as the number of playback signal frames
         * sent */
        if (in->echo_reference != NULL) {
            prepro_wr_cnt = push_echo_reference(in, in->proc_frames_in);
        } else {
            prepro_wr_cnt = in->proc_frames_in;
        }

        /* in_buf.frameCount and out_buf.frameCount indicate respectively the
         * maximum number of frames to be consumed and produced by process() */
        in_buf.frameCount = prepro_wr_cnt;
        in_buf.s16 = in->proc_buf;
        out_buf.frameCount = frames - frames_wr;
        out_buf.s16 = (int16_t *)buffer + frames_wr * in->config.channels;

        for (i = 0; i < in->num_preprocessors; i++)
            (*in->preprocessors[i])->process(in->preprocessors[i],
                                               &in_buf,
                                               &out_buf);

        /* process() has updated the number of frames consumed and produced in
         * in_buf.frameCount and out_buf.frameCount respectively
         * move remaining frames to the beginning of in->proc_buf */
        in->proc_frames_in -= in_buf.frameCount;
        if (in->proc_frames_in) {
            memcpy(in->proc_buf,
                   in->proc_buf + in_buf.frameCount * in->config.channels,
                   in->proc_frames_in * in->config.channels * sizeof(int16_t));
        }

        /* if not enough frames were passed to process(), read more and retry. */
        if (out_buf.frameCount == 0)
            continue;

        frames_wr += out_buf.frameCount;
    }
    return frames_wr;
}

static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
    int ret = 0;
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    struct omap_audio_device *adev = in->dev;
    size_t frames_rq = bytes / audio_stream_frame_size(&stream->common);

    LOGFUNC("%s(%p, %p, %d)", __FUNCTION__, stream, buffer, bytes);

    /* acquiring hw device mutex systematically is useful if a low priority thread is waiting
     * on the input stream mutex - e.g. executing select_mode() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->standby) {
        ret = start_input_stream(in);
        if (ret == 0)
            in->standby = 0;
    }
    pthread_mutex_unlock(&adev->lock);

    if (ret < 0)
        goto exit;

    if (in->num_preprocessors != 0)
        ret = process_frames(in, buffer, frames_rq);
    else if (in->resampler != NULL || in->remix_at_driver)
        ret = read_frames(in, buffer, frames_rq);
    else
        ret = pcm_read(in->pcm, buffer, bytes);

    if (ret > 0)
        ret = 0;

    if (ret == 0 && adev->mic_mute)
        memset(buffer, 0, bytes);

exit:
    if (ret < 0)
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               in_get_sample_rate(&stream->common));

    pthread_mutex_unlock(&in->lock);
    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    LOGFUNC("%s(%p)", __FUNCTION__, stream);

    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream,
                               effect_handle_t effect)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    int status;
    effect_descriptor_t desc;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, stream, effect);

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->num_preprocessors >= MAX_PREPROCESSORS) {
        status = -ENOSYS;
        goto exit;
    }

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
        goto exit;

    in->preprocessors[in->num_preprocessors++] = effect;

    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0) {
        in->need_echo_reference = true;
        do_input_standby(in);
    }

exit:

    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);
    return status;
}

static int in_remove_audio_effect(const struct audio_stream *stream,
                                  effect_handle_t effect)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;
    int i;
    int status = -EINVAL;
    bool found = false;
    effect_descriptor_t desc;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, stream, effect);

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->num_preprocessors <= 0) {
        status = -ENOSYS;
        goto exit;
    }

    for (i = 0; i < in->num_preprocessors; i++) {
        if (found) {
            in->preprocessors[i - 1] = in->preprocessors[i];
            continue;
        }
        if (in->preprocessors[i] == effect) {
            in->preprocessors[i] = NULL;
            status = 0;
            found = true;
        }
    }

    if (status != 0)
        goto exit;

    in->num_preprocessors--;

    status = (*effect)->get_descriptor(effect, &desc);
    if (status != 0)
        goto exit;
    if (memcmp(&desc.type, FX_IID_AEC, sizeof(effect_uuid_t)) == 0) {
        in->need_echo_reference = false;
        do_input_standby(in);
    }

exit:

    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);
    return status;
}


static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out)
{
    struct omap_audio_device *ladev = (struct omap_audio_device *)dev;
    struct omap_stream_out *out;
    int ret;

    LOGFUNC("%s(%p)", __FUNCTION__, dev);

    out = (struct omap_stream_out *)calloc(1, sizeof(struct omap_stream_out));
    if (!out)
        return -ENOMEM;
    if (devices & AUDIO_DEVICE_OUT_ALL_SCO) {
        ret = create_resampler(DEFAULT_OUT_SAMPLING_RATE,
                MM_FULL_POWER_SAMPLING_RATE,
                2,
                RESAMPLER_QUALITY_DEFAULT,
                NULL,
                &out->resampler);
        if (ret != 0)
            goto err_open;
        out->buffer = malloc(RESAMPLER_BUFFER_SIZE); /* todo: allow for reallocing */
    } else
       out->resampler = NULL;

    out->stream.common.get_sample_rate = out_get_sample_rate;
    out->stream.common.set_sample_rate = out_set_sample_rate;
    out->stream.common.get_buffer_size = out_get_buffer_size;
    out->stream.common.get_channels = out_get_channels;
    out->stream.common.get_format = out_get_format;
    out->stream.common.set_format = out_set_format;
    out->stream.common.standby = out_standby;
    out->stream.common.dump = out_dump;
    out->stream.common.set_parameters = out_set_parameters;
    out->stream.common.get_parameters = out_get_parameters;
    out->stream.common.add_audio_effect = out_add_audio_effect;
    out->stream.common.remove_audio_effect = out_remove_audio_effect;
    out->stream.get_latency = out_get_latency;
    out->stream.set_volume = out_set_volume;
    out->stream.write = out_write;
    out->stream.get_render_position = out_get_render_position;

    out->config = pcm_config_mm;

    out->dev = ladev;
    out->standby = 1;

    /* FIXME: when we support multiple output devices, we will want to
     * do the following:
     * adev->out_device = out->device;
     * select_output_device(adev);
     * This is because out_set_parameters() with a route is not
     * guaranteed to be called after an output stream is opened. */

    config->format = out_get_format(&out->stream.common);
    config->channel_mask = out_get_channels(&out->stream.common);
    config->sample_rate = out_get_sample_rate(&out->stream.common);

    *stream_out = &out->stream;
    return 0;

err_open:
    free(out);
    *stream_out = NULL;
    return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    struct omap_stream_out *out = (struct omap_stream_out *)stream;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, dev, stream);

    out_standby(&stream->common);
    if (out->buffer)
        free(out->buffer);
    if (out->resampler)
        release_resampler(out->resampler);
    free(stream);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)dev;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret;

    LOGFUNC("%s(%p, %s)", __FUNCTION__, dev, kvpairs);

    parms = str_parms_create_str(kvpairs);
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_TTY_MODE, value, sizeof(value));
    if (ret >= 0) {
        int tty_mode;

        if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_OFF) == 0)
            tty_mode = TTY_MODE_OFF;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_VCO) == 0)
            tty_mode = TTY_MODE_VCO;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_HCO) == 0)
            tty_mode = TTY_MODE_HCO;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_FULL) == 0)
            tty_mode = TTY_MODE_FULL;
        else
            return -EINVAL;

        pthread_mutex_lock(&adev->lock);
        if (tty_mode != adev->tty_mode) {
            adev->tty_mode = tty_mode;
            if (adev->mode == AUDIO_MODE_IN_CALL)
                select_output_device(adev);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_BT_NREC, value, sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, AUDIO_PARAMETER_VALUE_ON) == 0)
            adev->bluetooth_nrec = true;
        else
            adev->bluetooth_nrec = false;

    }

    str_parms_destroy(parms);
    return ret;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
    LOGFUNC("%s(%p, %s)", __FUNCTION__, dev, keys);

    return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    LOGFUNC("%s(%p)", __FUNCTION__, dev);
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)dev;

    LOGFUNC("%s(%p, %f)", __FUNCTION__, dev, volume);

    return 0;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    LOGFUNC("%s(%p, %f)", __FUNCTION__, dev, volume);

    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, int mode)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)dev;

    LOGFUNC("%s(%p, %d)", __FUNCTION__, dev, mode);

    pthread_mutex_lock(&adev->lock);
    if (adev->mode != mode) {
        adev->mode = mode;
        select_mode(adev);
    }
    pthread_mutex_unlock(&adev->lock);

    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)dev;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, dev, state);

    adev->mic_mute = state;

    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)dev;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, dev, state);

    *state = adev->mic_mute;

    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    size_t size;
    int channel_count = popcount(config->channel_mask);
    LOGFUNC("%s(%p, %d, %d, %d)", __FUNCTION__, dev, config->sample_rate,
            config->format, channel_count);

    return get_input_buffer_size(config->sample_rate, config->format, channel_count);
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in)
{
    struct omap_audio_device *ladev = (struct omap_audio_device *)dev;
    struct omap_stream_in *in;
    int ret;
    int channel_count = popcount(config->channel_mask);
    /*audioflinger expects return variable to be NULL incase of failure */
    *stream_in = NULL;
    LOGFUNC("%s(%p)", __FUNCTION__, dev);

    if (check_input_parameters(config->sample_rate, config->format, channel_count) != 0)
        return -EINVAL;

    in = (struct omap_stream_in *)calloc(1, sizeof(struct omap_stream_in));
    if (!in)
        return -ENOMEM;

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;
    in->remix_at_driver = NULL;

    in->requested_rate = config->sample_rate;

    memcpy(&in->config, &pcm_config_mm_ul, sizeof(pcm_config_mm_ul));
    in->config.channels = channel_count;

    in->buffer = malloc(2 * in->config.period_size * audio_stream_frame_size(&in->stream.common));
    if (!in->buffer) {
        ret = -ENOMEM;
        goto err;
    }

    if (in->requested_rate != in->config.rate) {
        in->buf_provider.get_next_buffer = get_next_buffer;
        in->buf_provider.release_buffer = release_buffer;
        ret = create_resampler(in->config.rate,
                               in->requested_rate,
                               in->config.channels,
                               RESAMPLER_QUALITY_DEFAULT,
                               &in->buf_provider,
                               &in->resampler);
        if (ret != 0) {
            ret = -EINVAL;
            goto err;
        }
    }

    in->dev = ladev;
    in->standby = 1;
    in->device = devices;

    *stream_in = &in->stream;
    return 0;

err:
    if (in->resampler)
        release_resampler(in->resampler);

    free(in);
    *stream_in = NULL;
    return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    struct omap_stream_in *in = (struct omap_stream_in *)stream;

    LOGFUNC("%s(%p, %p)", __FUNCTION__, dev, stream);

    in_standby(&stream->common);

    if (in->resampler) {
        free(in->buffer);
        release_resampler(in->resampler);
    }

    if (in->remix_at_driver)
        free(in->remix_at_driver);

    free(stream);
    return;
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    LOGFUNC("%s(%p, %d)", __FUNCTION__, device, fd);

    return 0;
}

static int adev_close(hw_device_t *device)
{
    struct omap_audio_device *adev = (struct omap_audio_device *)device;

    LOGFUNC("%s(%p)", __FUNCTION__, device);

    mixer_close(adev->mixer);
    free(device);
    return 0;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    struct omap_audio_device *adev;
    int ret;
    pthread_mutexattr_t mta;

    LOGFUNC("%s(%p, %s, %p)", __FUNCTION__, module, name, device);

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct omap_audio_device));
    if (!adev)
        return -ENOMEM;

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->hw_device.common.module = (struct hw_module_t *) module;
    adev->hw_device.common.close = adev_close;

    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;

    adev->mixer = mixer_open(0);
    if (!adev->mixer) {
        free(adev);
        ALOGE("Unable to open the mixer, aborting.");
        return -EINVAL;
    }

    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&adev->lock, &mta);
    pthread_mutexattr_destroy(&mta);

    /* Set the default route before the PCM stream is opened */
    pthread_mutex_lock(&adev->lock);
    set_route_by_array(adev->mixer, route_speaker);
    adev->mode = AUDIO_MODE_NORMAL;
    adev->out_device = AUDIO_DEVICE_OUT_SPEAKER;
    adev->in_device = AUDIO_DEVICE_IN_BUILTIN_MIC & ~AUDIO_DEVICE_BIT_IN;
    select_output_device(adev);

    adev->tty_mode = TTY_MODE_OFF;

    adev->input_requires_stereo = 0;
    adev->bluetooth_nrec = true;
    pthread_mutex_unlock(&adev->lock);
    *device = &adev->hw_device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Black Audio HAL",
        .author = "Texas Instruments",
        .methods = &hal_module_methods,
    },
};
