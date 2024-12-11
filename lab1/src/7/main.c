#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "uthreadpool.h"

#define TASKS 100

int cnt = 0;

void *foo(void *arg) {
    cnt++;
    // sleep(1);
    cnt++;
    return NULL;
}

void *foo_with_scheduling(void *arg) {
    uthread_scheduler_t *scheduler = arg;
    cnt++;
    usched_yield(scheduler);
    cnt++;
    return NULL;
}

int main(void) {
    uthread_scheduler_t *scheduler = init_scheduler();

    for (int i = 0; i < TASKS; i++) {
        add_to_scheduler(scheduler, foo_with_scheduling, scheduler);
    }

    run_scheduler(scheduler);

    printf("COUNT = %d\n", cnt);

    // reset_scheduler(scheduler);

    destroy_scheduler(&scheduler);
}
