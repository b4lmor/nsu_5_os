#include "routine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../list/list.h"

typedef enum compare_type {
    ASC, DESC, EQ
} __compare_type_t;

void *__compare_routine(void *args, __compare_type_t compare_type, int *iter, int *count);

int asc_count = 0;
int asc_iter = 0;

int desc_count = 0;
int desc_iter = 0;

int eq_count = 0;
int eq_iter = 0;

int swap_count = 0;
int swap_iter = 0;

void *asc_routine(void *args) {
    return __compare_routine(args, ASC, &asc_iter, &asc_count);
}

void *desc_routine(void *args) {
    return __compare_routine(args, DESC, &desc_iter, &desc_count);
}

void *eq_routine(void *args) {
    return __compare_routine(args, EQ, &eq_iter, &eq_count);
}

void *swap_routine(void *args) {
    linked_list_t *ll = args;

    while (!ll->stop) {
        read_lock(&ll->first->lock);

        node_t *prev = ll->first, *cur;
        while (prev->next != NULL) {
            if (rand() % 100 != 0) {
                cur = prev->next;

                read_lock(&cur->lock);

                unlock(&prev->lock);

                prev = cur;
                continue;
            }

            cur = prev->next;

            write_lock(&cur->lock);

            node_t *next = cur->next;
            if (next == NULL) {
                unlock(&cur->lock);
                break;
            }

            write_lock(&next->lock);

            prev->next = next;

            unlock(&prev->lock);

            cur->next = next->next;

            unlock(&cur->lock);

            next->next = cur;
            swap_count++;
            prev = next;

            unlock(&prev->lock);

            read_lock(&prev->lock);
        }

        unlock(&prev->lock);

        swap_iter++;
    }
}

void *print_routine(void *args) {
    while (!((linked_list_t *) args)->stop) {
        sleep(1);
        printf("asc: %d/%d,\t\tdesc: %d/%d,\t\teq: %d/%d,\t\tswap: %d/%d\n",
               asc_count, asc_iter,
               desc_count, desc_iter,
               eq_count, eq_iter,
               swap_count, swap_iter
        );
    }
    return NULL;
}

void *__compare_routine(void *args, __compare_type_t compare_type, int *iter, int *count) {
    const linked_list_t *ll = args;
    while (!ll->stop) {
        read_lock(&ll->first->lock);
        node_t *prev = ll->first;
        while (prev->next != NULL) {
            node_t *cur = prev->next;
            const int size = strnlen(prev->val, MAX_STRING_LENGTH);

            read_lock(&cur->lock);
            unlock(&prev->lock);

            switch (compare_type) {
                case ASC:
                    if (size < strlen(cur->val)) {
                        (*count)++;
                    }
                    break;
                case DESC:
                    if (size > strlen(cur->val)) {
                        (*count)++;
                    }
                    break;
                case EQ:
                    if (size == strlen(cur->val)) {
                        (*count)++;
                    }
                    break;
            }

            prev = cur;
        }

        unlock(&prev->lock);

        (*iter)++;
    }
    return NULL;
}
