#include <pthread.h>
extern float average;

extern pthread_mutex_t mutex;
void calculateEMA(float data[], float ema[], int size, float alpha);
