#ifndef MUTEX_H
#define MUTEX_H

#include <stdatomic.h>

typedef struct _mutex {
    int lock;
    int tid;
} my_mutex_t;

void mutex_init(my_mutex_t *m);
void mutex_lock(my_mutex_t *m);
void mutex_unlock(my_mutex_t *m);

#endif //SPINLOCK_H
