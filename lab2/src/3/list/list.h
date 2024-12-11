#ifndef LIST_H
#define LIST_H

#include "../lock/lock.h"

#define MAX_STRING_LENGTH 100
#define STORAGE_SIZE 1000

typedef struct _node_t {
    struct _node_t *next;
    custom_lock_t lock;
    char val[MAX_STRING_LENGTH];
} node_t;

typedef struct _linked_list_t {
    node_t *first;
    char stop;
} linked_list_t;

node_t *create_node(const char *val);

void linked_list_destroy(linked_list_t *ll);

#endif
