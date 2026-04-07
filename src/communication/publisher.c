#include "publisher.h"
#include <string.h>
#include <unistd.h>

void publisher_send_avg_int(PublisherState *state, int avg) {
    char msg[64];
    snprintf(msg, sizeof(msg), "audio.avg_int %d", avg);
    zmq_send(state->pub, msg, strlen(msg), 0);
}

PublisherState publisher_init(void) {
    void *_ctx = zmq_ctx_new();
    void *_pub = zmq_socket(_ctx, ZMQ_PUB);
    PublisherState state = {0};
    state.ctx = _ctx;
    state.pub = _pub;
    unlink("/run/cardash/bus.sock");
    zmq_bind(state.pub, "ipc:///run/cardash/bus.sock");
    return state;
}

void publisher_cleanup(PublisherState *state) {
    zmq_close(state->pub);
    zmq_ctx_destroy(state->ctx);
}
