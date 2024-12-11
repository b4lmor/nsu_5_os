#ifndef SRC3_ROUTINE_LOCK_H_
#define SRC3_ROUTINE_LOCK_H_

#include <pthread.h>

typedef struct custom_lock {
#ifdef USE_SPINLOCK
    pthread_spinlock_t sync;
#elifdef USE_MUTEX
    pthread_mutex_t sync;
#elifdef USE_RWLOCK
    pthread_rwlock_t sync;
#endif
} custom_lock_t;

void read_lock(custom_lock_t *lock);

void write_lock(custom_lock_t *lock);

void unlock(custom_lock_t *lock);

void lock_init(custom_lock_t *lock);

#endif
