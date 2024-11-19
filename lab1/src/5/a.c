#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

void print_sigmask() {
    sigset_t current_mask;

    if (sigprocmask(0, NULL, &current_mask) == -1) {
        perror("sigprocmask");
        return;
    }

    for (int i = 1; i < NSIG; i++) {
        if (sigismember(&current_mask, i)) {
            printf("Сигнал %d заблокирован\n", i);
        }
    }
}

void* block_signals(void* arg) {
    sigset_t block_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    sigaddset(&block_set, SIGQUIT);

    pthread_sigmask(SIG_BLOCK, &block_set, NULL);

    printf("Thread 1: Signals blocked (SIGINT and SIGQUIT)\n");
    while (1) {
        sleep(1);
    }
    return NULL;
}

void sigint_handler(int signum) {
    printf("Thread 2: Caught SIGINT\n");
}

void* handle_sigint(void* arg) {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);

    printf("Thread 2: Waiting for SIGINT...\n");
    while (1) {
        pause();
    }
    return NULL;
}

void* wait_sigquit(void* arg) {
    sigset_t wait_set;
    int sig;

    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGQUIT);

    printf("Thread 3: Waiting for SIGQUIT...\n");
    while (1) {
        sigwait(&wait_set, &sig);
        printf("Thread 3: Caught SIGQUIT\n");
    }
    return NULL;
}

void* wait_sigusr1(void* arg) {
    sigset_t wait_set;
    int sig;

    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGUSR1);

    printf("Thread 4: Waiting for SIGUSR1...\n");
    while (1) {
        sigwait(&wait_set, &sig);
        printf("Thread 4: Caught SIGUSR1\n");
    }
    return NULL;
}

int main() {
    print_sigmask();

    printf("main [%d %d]: Hello from main!\n", getpid(), getppid());

    pthread_t thread1, thread2, thread3, thread4;

    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &block_set, NULL);

    pthread_create(&thread1, NULL, block_signals, NULL);
    pthread_create(&thread2, NULL, handle_sigint, NULL);

    kill(getpid(), SIGQUIT);
    sleep(1);
    kill(getpid(), SIGUSR1);
    sleep(3);

    pthread_create(&thread3, NULL, wait_sigquit, NULL);
    pthread_create(&thread4, NULL, wait_sigusr1, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    // kill(getpid(), SIGINT);
    // sleep(1000);

    return EXIT_SUCCESS;
}
