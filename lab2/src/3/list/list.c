#include "list.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

node_t *create_node(const char *val) {
    node_t *node = malloc(sizeof(node_t));
    strncpy(node->val, val, MAX_STRING_LENGTH);
    node->next = NULL;
    lock_init(&node->lock);
    return node;
}

void linked_list_destroy(linked_list_t *ll) {
    node_t *cur = ll->first;
    while (cur->next != NULL) {
        node_t *tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    free(cur);
}
