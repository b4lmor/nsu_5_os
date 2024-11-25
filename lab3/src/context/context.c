#define _GNU_SOURCE
#include "../../include/context.h"
#include <stdio.h>
#include <stdlib.h>

proxy_context_t *init_context(const size_t mutex_number) {
    proxy_context_t *context = malloc(sizeof(proxy_context_t));
    if (context == NULL) {
        return NULL;
    }
    context->map = create_table();
    if (context->map == NULL) {
        perror("create_map");
        return NULL;
    }
    return context;
}

void destroy_context(proxy_context_t **context) {
    if (context == NULL || *context == NULL) {
        return;
    }
    destroy_hashmap(&(*context)->map);
    free(*context);
    *context = NULL;
}
