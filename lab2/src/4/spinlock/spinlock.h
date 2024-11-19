#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdatomic.h>

typedef int my_spinlock_t;

void spinlock_init(my_spinlock_t *lock);

void spinlock_lock(my_spinlock_t *lock);

void spinlock_unlock(my_spinlock_t *lock);

#endif //SPINLOCK_H
