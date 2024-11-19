#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 100
#define STORAGE_SIZE 1000

typedef struct _node_t {
    struct _node_t *next;
#ifdef USE_SPINLOCK
    pthread_spinlock_t sync;
#elifdef USE_MUTEX
    pthread_mutex_t sync;
#elifdef USE_RWLOCK
    pthread_rwlock_t sync;
#endif
    char val[MAX_STRING_LENGTH];
} node_t;

typedef struct _linked_list_t {
    node_t *first;
} linked_list_t;

node_t *create_node(const char *val);

void linked_list_destroy(linked_list_t *ll);

#endif
