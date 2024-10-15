#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <pipewire/impl.h>
#include "input/pipewire-input.h"
#include "main.h"

extern float average;
extern pthread_mutex_t mutex;
#define SAMPLE_SIZE 256 // samples per every output - more = more smoothnes
#define HZ 240
void *in_thread(void *arg) {
    pw_input_main_loop();
    pthread_exit(NULL);
}
void *out_thread(void *arg)
{
    int n = 0;
    u_int print_val = 0;
    float average_sum = 0.0f;
    float smooth[SAMPLE_SIZE];
    while (1)
    {
        usleep((int)(1000000 / (SAMPLE_SIZE * HZ))); 
        if(pthread_mutex_trylock(&mutex) != 0) {
            continue;
        }
        if (n >= SAMPLE_SIZE)
        {
            fflush(stdout);
            average_sum = 0.0f;
            for (int i = 0; i < SAMPLE_SIZE; i++)
            {
                average_sum += smooth[i];
            }
            n = 0;
            average_sum /= SAMPLE_SIZE;
        }
        printf("%d\n", (int)(average_sum*10000));
        smooth[n] = average;
        n++;
        pthread_mutex_unlock(&mutex);
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
