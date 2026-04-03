#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "input/pipewire-input.h"
#include "main.h"

extern float average;
extern pthread_mutex_t mutex;

#define SAMPLE_SIZE 16
#define HZ 500
#define SENSITIVITY 200

float smooth_output(float *raw_output, size_t size) {
    // Smooth the sampled output using exponential average

    static float prev_smooth = 1.0f;
    const float alpha = 0.1f;

    if (size == 0) return 0.0f;
    float smooth_value = 0.0f;
    for(size_t i = 0; i < size; i++) {
        smooth_value += raw_output[i];
    }
    // Remove weird artifacts
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
    float samples[SAMPLE_SIZE];
    float smooth_value;
    size_t bar_len;
    size_t prev_bar_len = 1;
    bool sleep_state = false;

    while(1)
    {
        if(!sleep_state) {
            usleep((int)(1000000 / (HZ * SAMPLE_SIZE)));
            if (n == SAMPLE_SIZE)
            {
                smooth_value = smooth_output(samples, SAMPLE_SIZE);
                bar_len = (size_t)(smooth_value * SENSITIVITY);
                print_bar(bar_len);
                n = 0;

                if(bar_len == 0 && prev_bar_len == 0) {
                    // Put the loop into sleep mode + reset sampling scope
                    n = 0;
                    printf("Put into sleep mode. average: %f\n", average);
                    sleep_state = true;
                    continue;
                }
                prev_bar_len = bar_len;
            }

            // Mutex scope
            if(pthread_mutex_trylock(&mutex) != 0) {
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
                usleep(10000);
            }
        }

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
