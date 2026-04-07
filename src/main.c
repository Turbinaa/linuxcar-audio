#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "input/pipewire-input.h"
#include "main.h"
#include "communication/publisher.h"

extern float average;
extern pthread_mutex_t mutex;

#define SAMPLE_SIZE 16
#define HZ 1000
#define SENSITIVITY 10000



float smooth_output(float *raw_output, size_t size) {
    // Smooth the sampled output using exponential average
    static float prev_smooth = 1.0f;
    const float alpha = 0.95f;

    if (size == 0) return prev_smooth;

    float smooth_value = 0.0f;
    for(size_t i = 0; i < size; i++) {
        prev_smooth = alpha * prev_smooth + (1 - alpha) * raw_output[i];
    }

    return prev_smooth;
}

void *in_thread(void *arg) {
    pw_input_main_loop();
    pthread_exit(NULL);
}

void *out_thread(void *arg)
{
    int n = 0;
    float samples[SAMPLE_SIZE];
    float smooth_value;
    size_t bar_len;
    size_t prev_bar_len = 1;
    bool sleep_state = false;
    PublisherState state = publisher_init();

    while(1)
    {
        if(!sleep_state) {
            usleep((int)(1000000 / (HZ * SAMPLE_SIZE)));
            if (n == SAMPLE_SIZE)
            {
                smooth_value = smooth_output(samples, SAMPLE_SIZE);
                bar_len = (size_t)(smooth_value * SENSITIVITY);
                n = 0;

                if(bar_len == 0 && prev_bar_len == 0) {
                    // Put the loop into sleep mode + reset sampling scope
                    n = 0;
                    sleep_state = true;
                    continue;
                }
                prev_bar_len = bar_len;
                publisher_send_avg_int(&state, (int)bar_len);
            }

            // Mutex scope
            if(pthread_mutex_trylock(&mutex) != 0) {
                usleep(1000);
                continue;
            }

            samples[n] = average;
            pthread_mutex_unlock(&mutex);
            // End of mutex scope

            n++; // Counter for sampling
        } else {
            // Periodically check if average is positive to revive the loop
            if(pthread_mutex_trylock(&mutex) == 0) {
                // Use smaller epsilon for more sensitivity
                if (average > 1e-8) {
                    pthread_mutex_unlock(&mutex);
                    sleep_state = false;
                } else {
                    pthread_mutex_unlock(&mutex);
                    usleep(250000); // 0.25s
                }
            } else {
                usleep(1000);
            }
        }

    }
    publisher_cleanup(&state);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
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
