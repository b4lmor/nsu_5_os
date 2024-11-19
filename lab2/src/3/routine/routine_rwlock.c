#ifdef USE_RWLOCK

#include "routine.h"
#include "../list/list.h"

int asc_count = 0;
int asc_iter = 0;

void *asc_routine(void *args) {
    linked_list_t *ll = args;

    while (true) {
        pthread_rwlock_rdlock(&ll->first->sync);
        node_t *prev = ll->first, *cur;
        while (prev->next != NULL) {
            cur = prev->next;
            int size = strlen(prev->val);
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_unlock(&prev->sync);
            if (size < strlen(cur->val)) {
                asc_count++;
            }
            prev = cur;
        }
        pthread_rwlock_unlock(&prev->sync);
        ++asc_iter;
    }
}

int desc_count = 0;
int desc_iter = 0;

void *desc_routine(void *args) {
    linked_list_t *ll = args;

    while (true) {
        pthread_rwlock_rdlock(&ll->first->sync);
        node_t *prev = ll->first, *cur;
        while (prev->next != NULL) {
            cur = prev->next;
            int size = strlen(prev->val);
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_unlock(&prev->sync);
            if (size > strlen(cur->val)) {
                desc_count++;
            }
            prev = cur;
        }
        pthread_rwlock_unlock(&prev->sync);
        ++desc_iter;
    }
}

int eq_count = 0;
int eq_iter = 0;

void *eq_routine(void *args) {
    linked_list_t *ll = args;

    while (true) {
        pthread_rwlock_rdlock(&ll->first->sync);
        node_t *prev = ll->first, *cur;
        while (prev->next != NULL) {
            cur = prev->next;
            int size = strlen(prev->val);
            pthread_rwlock_rdlock(&cur->sync);
            pthread_rwlock_unlock(&prev->sync);
            if (size == strlen(cur->val)) {
                eq_count++;
            }
            prev = cur;
        }
        pthread_rwlock_unlock(&prev->sync);
        ++eq_iter;
    }
}

int swap_count = 0;
int swap_iter = 0;

void *swap_routine(void *args) {
    linked_list_t *ll = args;

    while (true) {
        pthread_rwlock_rdlock(&ll->first->sync);
        node_t *prev = ll->first, *cur, *next;
        while (prev->next != NULL) {
            if (rand() % 100 != 0) {
                cur = prev->next;
                pthread_rwlock_rdlock(&cur->sync);
                pthread_rwlock_unlock(&prev->sync);
                prev = cur;
                continue;
            }
            cur = prev->next;
            pthread_rwlock_wrlock(&cur->sync);
            next = cur->next;
            if (next == NULL) {
                pthread_rwlock_unlock(&cur->sync);
                break;
            }
            pthread_rwlock_wrlock(&next->sync);
            prev->next = next;
            pthread_rwlock_unlock(&prev->sync);
            cur->next = next->next;
            pthread_rwlock_unlock(&cur->sync);
            next->next = cur;
            swap_count++;
            prev = next;
            pthread_rwlock_unlock(&prev->sync);
            pthread_rwlock_rdlock(&prev->sync);
        }
        pthread_rwlock_unlock(&prev->sync);
        ++swap_iter;
    }
}

void *print_routine(void *args) {
    while (true) {
        sleep(1);
        printf("asc: %d/%d,\t\tdesc: %d/%d,\t\teq: %d/%d,\t\tswap: %d/%d\n", asc_count, asc_iter, desc_count, desc_iter,
               eq_count, eq_iter, swap_count, swap_iter);
    }
}

#endif
