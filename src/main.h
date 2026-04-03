#include <pthread.h>
extern float average;

extern pthread_mutex_t mutex;

float smooth_output(float *raw_output, size_t size);

void print_bar(size_t bar_len);
