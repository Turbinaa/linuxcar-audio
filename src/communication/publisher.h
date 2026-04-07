#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <zmq.h>

typedef struct PublisherState {
    void *ctx;
    void *pub;
} PublisherState;

void publisher_send_avg_int(PublisherState *state, int avg);
PublisherState publisher_init(void);

void publisher_cleanup(PublisherState *state);
#endif
