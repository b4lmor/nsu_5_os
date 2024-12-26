#define _GNU_SOURCE
#include "../../include/context.h"
#include <stdio.h>
#include <stdlib.h>

proxy_context_t *init_context(const int port, const int upload_threads, const int handler_threads) {
    proxy_context_t *context = malloc(sizeof(proxy_context_t));
    if (context == NULL) {
        perror("malloc");
        return NULL;
    }

    context->stop = 0;
    context->port = port;

    context->map = create_table();
    if (context->map == NULL) {
        perror("Failed to create map table");
        free(context);
        return NULL;
    }

    context->handler_pool = init_threadpool(handler_threads);
    if (context->handler_pool == NULL) {
        perror("Failed to create handler pool");
        free(context);
        return NULL;
    }

    context->upload_pool = init_threadpool(upload_threads);
    if (context->upload_pool == NULL) {
        perror("Failed to create upload pool");
        free(context->handler_pool);
        free(context);
        return NULL;
    }

    return context;
}

void destroy_context(proxy_context_t **context) {
    if (context == NULL || *context == NULL) {
        return;
    }

    destroy_hashmap(&(*context)->map);

    destroy_threadpool(&(*context)->handler_pool);

    free(*context);
    *context = NULL;
}
