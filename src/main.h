#ifndef MAIN_H
#define MAIN_H
#include <pthread.h>
#include <signal.h>
extern float average;
extern pthread_mutex_t mutex;
extern volatile sig_atomic_t running;

float smooth_output(const float *raw_output, size_t size);

#endif
