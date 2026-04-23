#include "publisher.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void publisher_send_avg_int(PublisherState *state, int avg) {
    char msg[64];
    snprintf(msg, sizeof(msg), "audio.avg_int %d", avg);
    zmq_send(state->pub, msg, strlen(msg), 0);
}

PublisherState publisher_init(const char *socket_path) {
    void *_ctx = zmq_ctx_new();
    if (!_ctx) {
        fprintf(stderr, "zmq_ctx_new failed\n");
        exit(1);
    }
    void *_pub = zmq_socket(_ctx, ZMQ_PUB);
    if (!_pub) {
        fprintf(stderr, "zmq_socket failed\n");
        exit(1);
    }
    PublisherState state = {0};
    state.ctx = _ctx;
    state.pub = _pub;
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "ipc://%s", socket_path);

    if (zmq_bind(state.pub, full_path) != 0) {
        // try unlinking and retry, if fails again throw an error
        unlink(socket_path);

        if (zmq_bind(state.pub, full_path) != 0) {
            fprintf(stderr, "zmq_bind failed after unlink: %s\n", zmq_strerror(errno));
            exit(1);
        }
    }
    return state;
}

void publisher_cleanup(PublisherState *state) {
    zmq_close(state->pub);
    zmq_ctx_destroy(state->ctx);
}
