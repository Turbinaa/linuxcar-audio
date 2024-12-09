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
#define SAMPLES 256
#define HZ 500
void meanFilter(float signal[], int size, int window_size);
void *in_thread(void *arg) {
    pw_input_main_loop();
    pthread_exit(NULL);
}
void *out_thread(void *arg)
{
    int n = 0;
    float ema_data[SAMPLES];
    for(int i = 0; i < SAMPLES; i++) {
    	ema_data[i] = 0.0f;
    }
    while (1)
    {
        usleep((int)(1000000 / ( HZ * SAMPLES))); 
        if(pthread_mutex_trylock(&mutex) != 0) {
            continue;
        }
        if (n >= SAMPLES)
        {
            fflush(stdout);
			meanFilter(ema_data, SAMPLES, SAMPLES);
			for (int i = 0; i < SAMPLES; i++) {
				printf("%d\n", (int)(ema_data[i]*10000));
        		usleep((int)(1000000 / (HZ * SAMPLES))); 
			}

        	n = 0;

        }
		ema_data[n]=average;
        n++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void meanFilter(float signal[], int size, int window_size) {
    float filtered_signal[size];
    
    for (int i = 0; i < size; i++) {
        int start = i - window_size / 2;
        int end = i + window_size / 2;
        float sum = 0.0f;
    	int count = 0;
        for (int j = start; j <= end; j++) {
            if (j >= 0 && j < size) {
                sum += signal[j];
                count++;
            }
        }
        filtered_signal[i] = sum / count;
    }
    for (int i = 0; i < size; i++) {
        signal[i] = filtered_signal[i];
    }
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
