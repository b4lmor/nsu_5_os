#define _GNU_SOURCE
#include "../../include/context.h"
#include <stdio.h>
#include <stdlib.h>

proxy_context_t *init_context(const int port) {
    proxy_context_t *context = malloc(sizeof(proxy_context_t));
    if (context == NULL) {
        return NULL;
    }

    context->stop = 0;
    context->port = port;

    context->map = create_table();
    if (context->map == NULL) {
        free(context);
        return NULL;
    }

    context->pool = init_threadpool();
    if (context->pool == NULL) {
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

    destroy_threadpool(&(*context)->pool);

    free(*context);
    *context = NULL;
}
