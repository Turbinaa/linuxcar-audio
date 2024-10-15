#ifndef PIPEWIRE_INPUT_H
#define PIPEWIRE_INPUT_H

void process(void *userdata);
void pw_input_main_loop();
struct data
{
    struct pw_main_loop *loop;
    struct pw_stream *stream;
    struct spa_audio_info format;
};

void stream_param_changed(
    void *_data,
    uint32_t id,
    const struct spa_pod *param);
void quit(void *userdata, int sig);
#endif // PIPEWIRE_INPUT_H
