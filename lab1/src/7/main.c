#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "uthreadpool.h"

#define TASKS 10

typedef struct args {
    uthread_scheduler_t *scheduler;
    int id;
} args_t;

int cnt = 0;

void *foo_with_scheduling(void *arg) {
    args_t *args = arg;
    printf("%d -> ", args->id);
    cnt++;
    usched_yield(args->scheduler);
    printf("%d -> ", args->id);
    cnt++;
    return NULL;
}

int main(void) {
    uthread_scheduler_t *scheduler = init_scheduler();
    args_t args[TASKS];

    for (int i = 0; i < TASKS; i++) {
        args[i].scheduler = scheduler;
        args[i].id = i;
        add_to_scheduler(scheduler, foo_with_scheduling, &args[i]);
    }

    run_scheduler(scheduler);

    printf("\nCOUNT = %d\n", cnt);

    // reset_scheduler(scheduler);

    destroy_scheduler(&scheduler);
}
