#include "list.h"

node_t *create_node(const char *val) {
    node_t *node = malloc(sizeof(node_t));

    strncpy(node->val, val, MAX_STRING_LENGTH);
    node->next = NULL;

#ifdef USE_SPINLOCK
    pthread_spin_init(&node->sync, PTHREAD_PROCESS_SHARED);
#elifdef USE_MUTEX
    pthread_mutex_init(&node->sync, NULL);
#elifdef USE_RWLOCK
    pthread_rwlock_init(&node->sync, NULL);
#endif

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
