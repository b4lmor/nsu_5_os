#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sched.h>

#ifdef USE_QUEUE_SPINLOCK
#include "queue_spinlock/queue.h"
#elifdef USE_QUEUE_MUTEX
#include "queue_mutex/queue.h"
#elifdef USE_QUEUE_CONDITION_VARIABLE
#include "queue_condvar/queue.h"
#elifdef USE_QUEUE_SEMAPHORE
#include "queue_semaphore/queue.h"
#endif
//
// queue stats: current size 0; attempts: (127130 353399473 -353272343); counts (127130 127130 0)
// System time: 537 milliseconds
// User time:   7036 milliseconds

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

#define CPU_READER 1
#define CPU_WRITER 2

#define QUEUE_CAPACITY 1000000
#define CANCEL_TIME_SEC 2000
#define USE_SCHED_YIELD 1
#define PROFILE_TIME 1
#define EMULATE_SLEEP 1

void set_cpu(const int n) {
    cpu_set_t cpuset;
    const pthread_t tid = pthread_self();

    CPU_ZERO(&cpuset);
    CPU_SET(n, &cpuset);

    const int err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    if (err) {
        printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
        return;
    }

    printf("set_cpu: set cpu %d\n", n);
}

void *reader(void *arg) {
    int expected = 0;
    queue_t *q = arg;
    printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

    set_cpu(CPU_READER);

    while (1) {
#ifdef CANCEL_TIME_SEC
        pthread_testcancel();
#endif
        int val = -1;
        const int ok = queue_get(q, &val);
        if (!ok) {
            continue;
        }
        if (expected != val) {
            printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);
        }
        expected = val + 1;
    }
}

void *writer(void *arg) {
    int i = 0;
    queue_t *q = arg;
    printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

    set_cpu(CPU_WRITER);

    while (1) {
#if EMULATE_SLEEP
        usleep(1);
#endif
#ifdef CANCEL_TIME_SEC
        pthread_testcancel();
#endif
        const int ok = queue_add(q, i);
        if (!ok) {
            continue;
        }
        i++;
    }
}

void profile_time(void) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    const long user_time_ms = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
    const long sys_time_ms = usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000;
    printf("System time: %ld milliseconds\n", sys_time_ms);
    printf("User time:   %ld milliseconds\n", user_time_ms);
}

int main() {
    pthread_t rtid;
    pthread_t wtid;
    queue_t *q;

    printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

    q = queue_init(QUEUE_CAPACITY);

    int err = pthread_create(&rtid, NULL, reader, q);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        queue_destroy(q);
        return EXIT_FAILURE;
    }
#if USE_SCHED_YIELD
    sched_yield();
#endif
    err = pthread_create(&wtid, NULL, writer, q);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        queue_destroy(q);
        return EXIT_FAILURE;
    }

#ifndef CANCEL_TIME_SEC
    pthread_exit(NULL);
#else
    sleep(CANCEL_TIME_SEC);
    pthread_cancel(rtid);
    pthread_cancel(wtid);
    pthread_join(rtid, NULL);
    pthread_join(wtid, NULL);
#if PROFILE_TIME
    profile_time();
#endif

#endif
    queue_destroy(q);
    return EXIT_SUCCESS;
}
