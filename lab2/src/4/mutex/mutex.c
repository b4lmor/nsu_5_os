#define _GNU_SOURCE

#include <errno.h>
#include <linux/futex.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include "mutex.h"

#define STATUS_LOCK 0
#define STATUS_UNLOCK 1
#define NO_TID -1

int futex(void *uaddr, const int futex_op, const int val, const struct timespec *timeout, int *uaddr2, const int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int futex_wait(my_mutex_t *futex_addr) {
    const int err = futex(futex_addr, FUTEX_WAIT, STATUS_LOCK, NULL, NULL, 0);
    if (err < 0 && errno != EAGAIN) {
        printf("futex_wait [%d %d %d]: FUTEX_WAIT error: %s\n", getpid(), getppid(), gettid(), strerror(errno));
        return 1;
    }
    return 0;
}

int futex_wake(my_mutex_t *futex_addr) {
    const int futex_rc = futex(futex_addr, FUTEX_WAKE, 1, NULL, NULL, 0);
    if (futex_rc < 0 && errno != EAGAIN) {
        printf("futex_wake [%d %d %d]: FUTEX_WAKE error: %s\n", getpid(), getppid(), gettid(), strerror(errno));
        return 1;
    }
    return 0;
}

void mutex_init(my_mutex_t *m) {
    m->lock = 1;
    m->tid = NO_TID;
}

void mutex_lock(my_mutex_t *m) {
    while (1) {
        int expected = STATUS_UNLOCK;
        if (atomic_compare_exchange_strong(&m->lock, &expected, STATUS_LOCK))
            break;
        futex_wait(m);
    }
    m->tid = gettid();
}

void mutex_unlock(my_mutex_t *m) {
    if (m->tid != gettid()) {
        printf("mutex_unlock [%d %d %d]: not access\n", getpid(), getppid(), gettid());
        abort();
    }
    int expected = STATUS_LOCK;
    if (atomic_compare_exchange_strong(&m->lock, &expected, STATUS_UNLOCK)) {
        futex_wake(m);
    }
}
