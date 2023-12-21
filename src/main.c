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

#define SMOOTH_LEVEL 10
#define HZ 800
#define SCALE 30

typedef struct values
{
    int len;
    float value;
} Values;

// void draw(Block *block[SMOOTH_LEVEL])
// {
//     Block *_block = block;
//     float smooth[SMOOTH_LEVEL];
//     int _j = 0;
//     int k = 0;
//     if (n == SMOOTH_LEVEL)
//     {

//         log_smooth(smooth, SMOOTH_LEVEL);

//         for (int i = 0; i < SMOOTH_LEVEL; i++)
//         {
//             _block->block[i].key = &i;
//             _j = (int)ceilf(smooth[i] * 300);
//             for (int j = 0; j < _j; j++)
//             {
//                 if (i > 0 && (int)ceilf(smooth[i - 1] * 300) == _j)
//                     j++;
//                 k++;
//                 _block->block[i].len = k;
//                 fprintf(stdout, "─");
//             }

//             if (i - 1 < SMOOTH_LEVEL)
//                 printf("\n");
//             // printf("%f %f %d %d\n", average, smooth[i], n, i);
//         }
//         for (int i = 0; i < SMOOTH_LEVEL; i++)
//             _block->block[i].value = smooth[i];

//         fflush(NULL);
//     }
//     // ree(_block);
// }
void log_smooth(float data[], int size)
{
    for (int i = 0; i < size; i++)
    {
        data[i] = logf(data[i] + 1);
    }
}
void *in_thread(void *arg)
{
    pw_input_main_loop();
    pthread_exit(NULL);
}
void *out_thread(void *arg)
{
    int n = 0;
    int _j = 0;
    float average_sum = 0;
    float smooth[SMOOTH_LEVEL];
    float max_recorded_value = 0;
    int calculations = 0;
    int value = 0;
    int _value = 0;
    while (1)
    {
        printf("\n");
        pthread_mutex_lock(&mutex);
        if (n == SMOOTH_LEVEL)
        {
            _value = 0;
            value = 0;
            log_smooth(smooth, SMOOTH_LEVEL);

            // for (int i = 0; i < SMOOTH_LEVEL; i++)
            // {
            //     // _j = (int)ceilf(smooth[i] * 300);
            //     // for (int j = 0; j < _j; j++)
            //     // {
            //     //     if ((int)ceilf(smooth[i - 1] * 300) == _j && i > 0)
            //     //         j++;
            //     //     // fprintf(stdout, ".");
            //     // }
            //     // if (i + 1 < SMOOTH_LEVEL)
            //     //     fprintf(stdout, "\n");
            //     // printf("%f %f %d %d\n", average, smooth[i], n, i);
            // }
            for (int i = 0; i < SMOOTH_LEVEL; i++)
            {
                average_sum += smooth[i];
                if (smooth[i] > max_recorded_value && max_recorded_value >= 0)
                {
                    max_recorded_value = smooth[i];
                }
                else
                {
                    max_recorded_value -= max_recorded_value / (HZ * SMOOTH_LEVEL);
                }
            }
            average_sum = average_sum / SMOOTH_LEVEL;
            // printf("%f\n", average_sum);
            // max --         10
            // average_sum -- x    -- y
            calculations = (int)(((average_sum * (SCALE - 1)) / max_recorded_value));
            if (calculations > 0)
            {
                for (int i = 0; i < calculations; i++)
                {
                    printf(".");
                    value++;
                }
                // printf("\n");
            }
            fflush(NULL);
            average_sum = 0;
            n = 0;
        }
        // else
        // {
        //     for (int i = 0; i < SMOOTH_LEVEL; i++)
        //     {
        //         _j = (int)ceilf(values[i].value * 300);
        //         for (int j = 0; j < _j; j++)
        //         {
        //             if ((int)ceilf(values[i - 1].value * 300) == _j && i > 0)
        //                 j++;
        //             fprintf(stdout, ".");
        //         }
        //         if (i + 1 < SMOOTH_LEVEL)
        //             fprintf(stdout, "\n");
        //         // printf("%f %f %d %d\n", average, smooth[i], n, i);
        //     }

        smooth[n] = average;
        // for (int i = 0; i < _value; i++)
        // {
        //     printf(" ");
        // }
        // printf("*");
        // // if (_value > 0)
        // // printf("\n");
        // printf("\e[1;1H\e[2J");
        n++;
        // _value = floor((float)value * exp(-0.01 * n));
        pthread_mutex_unlock(&mutex);
        usleep(1000000 / HZ);
    }
    // for (int i = 0; i < SMOOTH_LEVEL; i++)
    // {
    //     free(_block->values[i]);
    // }
    // free(_block);
    pthread_exit(NULL);
}
int main()
{
    // fputs("\e[?25l", stdout);
    pthread_t thread_id[2];
    int ids[2] = {1, 2};
    pthread_create(&thread_id[0], NULL, in_thread, (void *)&ids[0]);
    pthread_create(&thread_id[1], NULL, out_thread, (void *)&ids[1]);

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    pthread_mutex_destroy(&mutex);
}
