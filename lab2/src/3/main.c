#include "list/list.h"
#include "routine/routine.h"

int main() {
    linked_list_t *ll = malloc(sizeof(linked_list_t));
    srand(time(NULL));
    char val[MAX_STRING_LENGTH] = {0};
    memset(val, '0', rand() % MAX_STRING_LENGTH);
    ll->first = create_node(val);

    node_t *last = ll->first;
    for (int i = 1; i < STORAGE_SIZE; ++i) {
        memset(val, 0, MAX_STRING_LENGTH);
        memset(val, '0', rand() % MAX_STRING_LENGTH);
        node_t *newNode = create_node(val);
        last->next = newNode;
        last = newNode;
    }

    pthread_t tid[7];
    void* (*routines[7])(void*) = {asc_routine, desc_routine, eq_routine, print_routine, swap_routine, swap_routine, swap_routine};
    void* args[7] = {ll, ll, ll, NULL, ll, ll, ll};

    for (int i = 0; i < 7; ++i) {
        if (pthread_create(&tid[i], NULL, routines[i], args[i])) {
            printf("main: pthread_create()\n");
            return EXIT_FAILURE;
        }
    }

    sleep(10);

    for (int i = 0; i < 7; ++i) {
        pthread_cancel(tid[i]);
    }

    linked_list_destroy(ll);
    free(ll);

    return EXIT_SUCCESS;
}
