#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>

#include "queue.h"

my_spinlock_t *spinlock_init() {
    my_spinlock_t *spinlock = malloc(sizeof(my_spinlock_t));
    if (!spinlock) {
        printf("Cannot allocate memory for a lock\n");
        abort();
    }
    spinlock->lock = false;
    return spinlock;
}

void spinlock_lock(my_spinlock_t *s) {
    while (__atomic_test_and_set(&s->lock, __ATOMIC_ACQUIRE));
}

void spinlock_unlock(my_spinlock_t *s) {
    __atomic_clear(&s->lock, __ATOMIC_RELEASE);
}

void *qmonitor(void *arg) {
    queue_t *q = arg;

    printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

    while (1) {
        queue_print_stats(q);
        sleep(1);
    }

    return NULL;
}

queue_t *queue_init(int max_count) {
    int err;

    queue_t *q = malloc(sizeof(queue_t));
    if (!q) {
        printf("Cannot allocate memory for a queue\n");
        abort();
    }
    q->lock = spinlock_init();
    q->first = NULL;
    q->last = NULL;
    q->max_count = max_count;
    q->count = 0;

    q->add_attempts = q->get_attempts = 0;
    q->add_count = q->get_count = 0;

    err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
    if (err) {
        printf("queue_init: pthread_create() failed: %s\n", strerror(err));
        abort();
    }

    return q;
}

void queue_destroy(queue_t *q) {
    if (!q) return;

    spinlock_lock(q->lock);

    pthread_cancel(q->qmonitor_tid);
    pthread_join(q->qmonitor_tid, NULL);
    while (q->first != NULL) {
        qnode_t *tmp = q->first;
        q->first = q->first->next;
        free(tmp);
    }
    free(q->lock);
    free(q);
    q = NULL;
}

int queue_add(queue_t *q, int val) {
    spinlock_lock(q->lock);

    q->add_attempts++;

    assert(q->count <= q->max_count);
    if (q->count == q->max_count) {
        spinlock_unlock(q->lock);
        return 0;
    }

    qnode_t *new = malloc(sizeof(qnode_t));
    if (!new) {
        printf("Cannot allocate memory for new node\n");
        abort();
    }
    new->val = val;
    new->next = NULL;

    if (!q->first) {
        // here we can miss a value
        q->first = q->last = new;
    } else {
        // bcz of that
        q->last->next = new;
        q->last = q->last->next;
    }

    q->count++;
    q->add_count++;

    spinlock_unlock(q->lock);
    return 1;
}

int queue_get(queue_t *q, int *val) {
    spinlock_lock(q->lock);

    q->get_attempts++;

    assert(q->count >= 0);
    if (q->count == 0) { // || q->first == NULL) // ???
        spinlock_unlock(q->lock);
        return 0;
    }

    qnode_t *tmp = q->first;

    *val = tmp->val;
    q->first = q->first->next;

    free(tmp);

    q->count--;
    q->get_count++;

    spinlock_unlock(q->lock);
    return 1;
}

void queue_print_stats(queue_t *q) {
    printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
           q->count,
           q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
           q->add_count, q->get_count, q->add_count - q->get_count
    );
}
