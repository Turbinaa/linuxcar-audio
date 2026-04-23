#include "main.h"

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "communication/publisher.h"
#include "input/pipewire-input.h"

extern float average;
extern pthread_mutex_t mutex;

volatile sig_atomic_t running = 1;

// Default values
static unsigned int sample_size = 16;
static unsigned int hz = 1000;
static unsigned int sensitivity = 10000;
static char *socket_path = "/run/cardash/bus.sock";

static void handle_signal(int sig) {
    running = 0;
}
float smooth_output(const float *raw_output, size_t size) {
    // Smooth the sampled output using exponential average
    static float prev_smooth = 1.0f;
    const float alpha = 0.95f;
    for (size_t i = 0; i < size; i++) {
        prev_smooth = alpha * prev_smooth + (1 - alpha) * raw_output[i];
    }

    return prev_smooth;
}

void *in_thread(void *arg) {
    pw_input_main_loop();
    pthread_exit(NULL);
}

void *out_thread(void *arg) {
    unsigned int n = 0;

    float *samples = malloc(sizeof(float) * sample_size);
    if (!samples)
        pthread_exit(NULL);

    bool sleep_state = false;
    PublisherState state = publisher_init(socket_path);

    while (running) {
        if (!sleep_state) {
            usleep((int)(1000000 / (hz * sample_size)));
            if (n == sample_size) {
                float smooth_value = smooth_output(samples, sample_size);
                int bar_len = (int)(smooth_value * sensitivity);
                // Hard limit
                if(bar_len > 1000) bar_len = 1000;

                if (bar_len == 0) {
                    // Put the loop into sleep mode + reset sampling scope
                    n = 0;
                    sleep_state = true;
                    continue;
                }
                publisher_send_avg_int(&state, (int)bar_len);
                n = 0;
            }

            // Mutex scope
            if (pthread_mutex_trylock(&mutex) != 0) {
                usleep(1000);
                continue;
            }

            samples[n] = average;
            pthread_mutex_unlock(&mutex);
            // End of mutex scope

            n++;  // Counter for sampling
        } else {
            // Periodically check if average is positive to revive the loop
            if (pthread_mutex_trylock(&mutex) == 0) {
                // Use smaller epsilon for more sensitivity
                if (average > 1e-8) {
                    pthread_mutex_unlock(&mutex);
                    sleep_state = false;
                } else {
                    pthread_mutex_unlock(&mutex);
                    usleep(250000);  // 0.25s
                }
            } else {
                usleep(1000);
            }
        }
    }
    free(samples);
    publisher_cleanup(&state);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-hz") == 0 && i + 1 < argc) {
            if (atoi(argv[i + 1]) > 0) {
                hz = (unsigned int)atoi(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "HZ argument (-hz) must be positive! Aborting.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-ss") == 0 && i + 1 < argc) {
            if (atoi(argv[i + 1]) > 0) {
                sample_size = (unsigned int)atoi(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Sample size (-ss) argument must be positive! Aborting.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-sn") == 0 && i + 1 < argc) {
            if (atoi(argv[i + 1]) > 0) {
                sensitivity = (unsigned int)atoi(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Sensitivity (-sn) argument must be positive! Aborting.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--socket") == 0 && i + 1 < argc) {
            socket_path = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Available arguments: \n");
            printf("Socket connection path: --socket <path-to-sock> (default = \"/run/cardash/bus.sock\")\n");
            printf("Sample rate: -hz <int> (default = 1000)\n");
            printf("Sample size: -ss <int> (default = 16)\n");
            printf("Sensitivity: -sn <int> (default = 10000)\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown Argument: %s\n", argv[i]);
            return -1;
        }
    }

    struct sigaction sa = {.sa_handler = handle_signal, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int ids[2] = {1, 2};
    pthread_t thread_id[2];

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&thread_id[0], NULL, in_thread, (void *)&ids[0]);
    pthread_create(&thread_id[1], NULL, out_thread, (void *)&ids[1]);

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}
