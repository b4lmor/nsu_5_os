#define _GNU_SOURCE
#include <pthread.h>
#include "spinlock/spinlock.h"
#include "mutex/mutex.h"

#define NUM_THREADS 20
#define INCREMENTS 1000000
#define NUM_CPU 4

int counter = 0;
my_spinlock_t spinlock;
my_mutex_t mutex;

void set_cpu(const int n) {
    cpu_set_t cpuset;
    const pthread_t tid = pthread_self();

    CPU_ZERO(&cpuset);
    CPU_SET(n, &cpuset);

    const int err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    if (err) {
        printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
    }
}

void* increment_with_spinlock(void* arg) {
    const int n = *(int*)arg % NUM_CPU;
    set_cpu(n);
    for (int i = 0; i < INCREMENTS; i++) {
        spinlock_lock(&spinlock);
        counter++;
        spinlock_unlock(&spinlock);
    }
    return NULL;
}

void* increment_with_mutex(void* arg) {
    const int n = *(int*)arg;
    set_cpu(n % NUM_CPU);
    for (int i = 0; i < INCREMENTS; i++) {
        mutex_lock(&mutex);
        counter++;
        mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    spinlock_init(&spinlock);
    counter = 0;
    int args[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = i;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        const int err = pthread_create(&threads[i], NULL, increment_with_spinlock, &args[i]);
        if (err != 0) {
            printf("Error creating thread %d\n", i);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Final counter value with spinlock: %d\n", counter);

    mutex_init(&mutex);
    counter = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        const int err = pthread_create(&threads[i], NULL, increment_with_mutex, &args[i]);
        if (err != 0) {
            printf("Error creating thread %d\n", i);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Final counter value with mutex: %d\n", counter);

    return EXIT_SUCCESS;
}
