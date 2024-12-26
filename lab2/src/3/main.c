#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list/list.h"
#include "routine/routine.h"

int main() {

#ifdef USE_SPINLOCK
    printf("spinlock\n");
#elifdef USE_MUTEX
    printf("mutex\n");
#elifdef USE_RWLOCK
    printf("rwlock\n");
#endif

    linked_list_t *ll = malloc(sizeof(linked_list_t));
    ll->stop = false;
    srand(time(NULL));
    char val[MAX_STRING_LENGTH] = {0};
    memset(val, 'x', rand() % (MAX_STRING_LENGTH - 1));
    ll->first = create_node(val);

    node_t *last = ll->first;
    for (int i = 1; i < STORAGE_SIZE; ++i) {
        memset(val, 0, MAX_STRING_LENGTH);
        memset(val, 'x', rand() % (MAX_STRING_LENGTH - 1));
        node_t *newNode = create_node(val);
        last->next = newNode;
        last = newNode;
    }

    pthread_t tid[7];
    void* (*routines[7])(void*) = {
        asc_routine,
        desc_routine,
        eq_routine,
        print_routine,
        swap_routine,
        swap_routine,
        swap_routine
    };

    int broken = 7;

    for (int i = 0; i < 7; i++) {
        if (pthread_create(&tid[i], NULL, routines[i], ll)) {
            printf("main: pthread_create()\n");
            broken = i;
            goto wait;
        }
    }

    sleep(10);
wait:
    ll->stop = true;
    for (int i = 0; i < broken; ++i) {
        pthread_join(tid[i], NULL);
    }

    linked_list_destroy(ll);

    return EXIT_SUCCESS;
}
