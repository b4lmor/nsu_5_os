cmake_minimum_required(VERSION 3.29)
project(os_sync C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(lab2_1_a src/1/a.c src/1/queue/queue.h src/1/queue/queue.c)
add_executable(lab2_1_b src/1/b.c src/1/queue/queue.h src/1/queue/queue.c)


add_executable(lab2_2_spinlock src/2/queue_threads.c src/2/queue_spinlock/queue.h src/2/queue_spinlock/queue.c)
target_compile_definitions(lab2_2_spinlock PRIVATE USE_QUEUE_SPINLOCK)

add_executable(lab2_2_mutex src/2/queue_threads.c src/2/queue_mutex/queue.h src/2/queue_mutex/queue.c)
target_compile_definitions(lab2_2_mutex PRIVATE USE_QUEUE_MUTEX)

add_executable(lab2_2_condvar src/2/queue_threads.c src/2/queue_condvar/queue.h src/2/queue_condvar/queue.c)
target_compile_definitions(lab2_2_condvar PRIVATE USE_QUEUE_CONDITION_VARIABLE)

add_executable(lab2_2_semaphore src/2/queue_threads.c src/2/queue_semaphore/queue.h src/2/queue_semaphore/queue.c)
target_compile_definitions(lab2_2_semaphore PRIVATE USE_QUEUE_SEMAPHORE)


add_executable(lab2_3_spinlock src/3/main.c
        src/3/list/list.h src/3/list/list.c
        src/3/routine/routine.h src/3/routine/routine.c
        src/3/lock/lock.h src/3/lock/lock.c)
target_compile_definitions(lab2_3_spinlock PRIVATE USE_SPINLOCK)

add_executable(lab2_3_mutex src/3/main.c
        src/3/list/list.h src/3/list/list.c
        src/3/routine/routine.h src/3/routine/routine.c
        src/3/lock/lock.h src/3/lock/lock.c)
target_compile_definitions(lab2_3_mutex PRIVATE USE_MUTEX)

add_executable(lab2_3_rwlock src/3/main.c src/3/main.c
        src/3/list/list.h src/3/list/list.c
        src/3/routine/routine.h src/3/routine/routine.c
        src/3/lock/lock.h src/3/lock/lock.c)
target_compile_definitions(lab2_3_rwlock PRIVATE USE_RWLOCK)


add_executable(lab2_4 src/4/test.c src/4/spinlock/spinlock.h src/4/spinlock/spinlock.c src/4/mutex/mutex.h src/4/mutex/mutex.c)
