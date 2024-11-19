#define _GNU_SOURCE
#include "../../include/context.h"
#include <stdio.h>
#include <stdlib.h>

Context *init_context(const size_t mutex_number) {
    Context *context = malloc(sizeof(Context));
    if (context == NULL) {
        return NULL;
    }
    context->last_cached_urls = malloc(sizeof(char *) * mutex_number);
    if (context->last_cached_urls == NULL) {
        free(context);
        return NULL;
    }
    context->mutex_number = mutex_number;
    context->mutexes = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * mutex_number);
    for (int i = 0; i < mutex_number; i++) {
        if (pthread_mutex_init(&context->mutexes[i], NULL) != 0) {
            fprintf(stderr, "Ошибка инициализации мьютекса %d\n", i);
            for (int j = 0; j < i; j++) {
                pthread_mutex_destroy(&context->mutexes[j]);
                free(context->last_cached_urls);
            }
            free(context->mutexes);
            free(context);
            return NULL;
        }
    }
    return context;
}

void destroy_context(Context **context) {
    if (*context == NULL) {
        return;
    }
    if ((*context)->mutexes != NULL) {
        free((*context)->mutexes);
    }
    free(*context);
    *context = NULL;
}
