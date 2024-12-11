#include "lock.h"

void read_lock(custom_lock_t *lock) {
#ifdef USE_SPINLOCK
    pthread_spin_lock(&lock->sync);
#elifdef USE_MUTEX
    pthread_mutex_lock(&lock->sync);
#elifdef USE_RWLOCK
    pthread_rwlock_rdlock(&lock->sync);
#endif
}

void write_lock(custom_lock_t *lock) {
#ifdef USE_SPINLOCK
    pthread_spin_lock(&lock->sync);
#elifdef USE_MUTEX
    pthread_mutex_lock(&lock->sync);
#elifdef USE_RWLOCK
    pthread_rwlock_wrlock(&lock->sync);
#endif
}

void unlock(custom_lock_t *lock) {
#ifdef USE_SPINLOCK
    pthread_spin_unlock(&lock->sync);
#elifdef USE_MUTEX
    pthread_mutex_unlock(&lock->sync);
#elifdef USE_RWLOCK
    pthread_rwlock_unlock(&lock->sync);
#endif
}

void lock_init(custom_lock_t *lock) {
#ifdef USE_SPINLOCK
    pthread_spin_init(&lock->sync, NULL);
#elifdef USE_MUTEX
    pthread_mutex_init(&lock->sync, NULL);
#elifdef USE_RWLOCK
    pthread_rwlock_init(&lock->sync, NULL);
#endif
}

