#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <pipewire/impl.h>

#include "pipewire-input.h"
#include "../main.h"

float average;
pthread_mutex_t mutex;
static const struct pw_stream_events stream_e = {
    PW_VERSION_STREAM_EVENTS,
    .param_changed = stream_param_changed,
    .process = process,

};
void pw_input_main_loop()
{
    struct data data = {
        0,
    };
    const struct spa_pod *params[1];
    uint8_t buffer[1024];
    struct pw_properties *props;
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    pw_init(NULL, NULL);
    data.loop = pw_main_loop_new(NULL);
    pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGINT, quit, &data);
    pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGTERM, quit, &data);
    props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                              PW_KEY_CONFIG_NAME, "client-rt.conf",
                              PW_KEY_MEDIA_CATEGORY, "Capture",
                              PW_KEY_MEDIA_ROLE, "Music",
                              NULL);
    pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true");
    pw_properties_set(props, PW_KEY_STREAM_MONITOR, "true");
    data.stream = pw_stream_new_simple(
        pw_main_loop_get_loop(data.loop),
        "Capture audio",
        props,
        &stream_e,
        &data);
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                           &SPA_AUDIO_INFO_RAW_INIT(
                                                   .format = SPA_AUDIO_FORMAT_F32));
    pw_stream_connect(data.stream,
                      PW_DIRECTION_INPUT,
                      PW_ID_ANY,
                      PW_STREAM_FLAG_AUTOCONNECT |
                          PW_STREAM_FLAG_MAP_BUFFERS |
                          PW_STREAM_FLAG_RT_PROCESS,
                      params, 1);

    pw_main_loop_run(data.loop);

    pw_stream_destroy(data.stream);
    pw_main_loop_destroy(data.loop);
    pw_deinit();
}

void process(void *userdata)
{
    struct data *data = userdata;
    struct spa_buffer *spa_buff;
    struct pw_buffer *pw_buff;
    float *samples;

    uint32_t j, n, n_channels, n_samples;

    pw_buff = pw_stream_dequeue_buffer(data->stream);
    if (pw_buff == NULL)
    {
        fprintf(stderr, "Out of buffers, aborting");
        return;
    }
    spa_buff = pw_buff->buffer;
    samples = spa_buff->datas[0].data;
    if (samples == NULL)
        return;

    n_channels = data->format.info.raw.channels;
    n_samples = spa_buff->datas[0].chunk->size / sizeof(float);

    if(pthread_mutex_trylock(&mutex) != 0) {
        average = 0.0f;
        for (j = 0; j < n_channels; j++) {
            for (n = j; n < n_samples; n += n_channels)
            {
                average += fabsf(samples[n]);
            }
        }
        average /= n_samples;
        average /= n_channels;
        pthread_mutex_unlock(&mutex);
    }
    pw_stream_queue_buffer(data->stream, pw_buff);
}

void stream_param_changed(
    void *_data,
    uint32_t id,
    const struct spa_pod *p)
{
    struct data *data = _data;
    if (p == NULL)
        return;
    if (spa_format_parse(p, &data->format.media_type, &data->format.media_subtype) < 0)
        return;
    // raw audio only
    if (data->format.media_type != SPA_MEDIA_TYPE_audio ||
        data->format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
        return;

    spa_format_audio_raw_parse(p, &data->format.info.raw);
    fprintf(stdout, "Capturing audio at: channels: %d, rate: %d ", data->format.info.raw.channels, data->format.info.raw.rate);
}
void quit(void *userdata, int sig)
{
    const struct data *data = userdata;
    pw_main_loop_quit(data->loop);
}
