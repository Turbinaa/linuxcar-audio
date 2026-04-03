#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <pipewire/impl.h>
#include "input/pipewire-input.h"
#include "main.h"

extern float average;
extern pthread_mutex_t mutex;

#define SAMPLE_SIZE 16
#define HZ 1000

float smooth_output(float *raw_output, size_t size) {
    // Smooth the sampled output using exponential average

    static float prev_smooth = 1.0f;
    const float alpha = 0.01f;

    if (size == 0) return 0.0f;
    float smooth_value = 0.0f;
    for(size_t i = 0; i < size; i++) {
        smooth_value += raw_output[i];
    }
    smooth_value = alpha * prev_smooth + (1 - alpha) * smooth_value;
    prev_smooth = smooth_value;
    return smooth_value;
}

void print_bar(size_t bar_len) {
    if(bar_len > 128)
        bar_len = 128;

    fflush(stdout);
    for(size_t i = 0; i < bar_len; i++) {
        putchar('#');
    }
    putchar('\n');
}
void *in_thread(void *arg) {
    pw_input_main_loop();
    pthread_exit(NULL);
}

void *out_thread(void *arg)
{
    int n = 0;
    u_int print_val = 0;
    float samples[SAMPLE_SIZE];

    float smooth_value;
    size_t bar_len;

    while(1)
    {
        usleep((int)(1000000 / (HZ * SAMPLE_SIZE)));

        if(pthread_mutex_trylock(&mutex) != 0) {
            continue;
        }

        if (n == SAMPLE_SIZE)
        {
            smooth_value = smooth_output(samples, SAMPLE_SIZE);
            bar_len = (size_t)(smooth_value * 1000);
            print_bar(bar_len);
            n = 0;
        }

        samples[n] = average;
        pthread_mutex_unlock(&mutex);
        n++;
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread_id[2];
    pthread_mutex_init(&mutex, NULL);
    int ids[2] = {1, 2};
    pthread_create(&thread_id[0], NULL, in_thread, (void *)&ids[0]);
    pthread_create(&thread_id[1], NULL, out_thread, (void *)&ids[1]);

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    pthread_mutex_destroy(&mutex);
}
