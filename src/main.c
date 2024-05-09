#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <pipewire/impl.h>
#include <math.h>
#include "input/pipewire-input.h"
#include "main.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
extern float average;

#define HZ 60
#define SMOOTH_LEVEL 256 // or sample size - higher the sample, smoother the output and less sensitiveness


void *in_thread(void *arg)
{
    pw_input_main_loop();
    pthread_exit(NULL);
}
void *out_thread(void *arg)
{
    int n = 0;
    float average_sum_prev = 0;
    float average_sum = 0;
    float smooth[SMOOTH_LEVEL];
    float max_recorded_value = 0;
    float max_recorded_value_prev = 0;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        usleep((int)(1000000 / (HZ * SMOOTH_LEVEL) )); 
        if (n >= SMOOTH_LEVEL)
        {
            for (int i = 0; i < SMOOTH_LEVEL; i++)
            {
                /*
               if (smooth[i] > max_recorded_value && max_recorded_value >= 0)
                {
                    max_recorded_value = smooth[i];
                }
                else
                {
                    max_recorded_value_prev = log(max_recorded_value_prev);
                    max_recorded_value -= (log(max_recorded_value) - max_recorded_value_prev) / max_recorded_value_prev;
                    max_recorded_value_prev = max_recorded_value;
                }
                */
                average_sum += smooth[i];
            }
            n = 0;
            average_sum = (average_sum / SMOOTH_LEVEL);
        }
        if(average_sum_prev > 0) {
            average_sum *= ((log(average_sum) - average_sum_prev) / average_sum_prev);
            average_sum_prev = average_sum;
        }
        printf("%f\n", average_sum);
        fflush(NULL);
        smooth[n] = average;
        n++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread_id[2];
    int ids[2] = {1, 2};
    pthread_create(&thread_id[0], NULL, in_thread, (void *)&ids[0]);
    pthread_create(&thread_id[1], NULL, out_thread, (void *)&ids[1]);

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    pthread_mutex_destroy(&mutex);
}
