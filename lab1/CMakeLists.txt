cmake_minimum_required(VERSION 3.29)
project(lab1 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab1_1_a src/1/a.c)
add_executable(lab1_1_b src/1/b.c)
add_executable(lab1_1_c_1 src/1/c_1.c)
add_executable(lab1_1_c_2 src/1/c_2.c)
add_executable(lab1_1_d src/1/d.c)
add_executable(lab1_1_e src/1/e.c)

add_executable(lab1_2_a src/2/a.c)
add_executable(lab1_2_b src/2/b.c)
add_executable(lab1_2_c src/2/c.c)
add_executable(lab1_2_d src/2/d.c)
add_executable(lab1_2_e src/2/e.c)
add_executable(lab1_2_f src/2/f.c)

add_executable(lab1_3_a src/3/a.c)
add_executable(lab1_3_b src/3/b.c)

add_executable(lab1_4_a src/4/a.c)
add_executable(lab1_4_b src/4/b.c)
add_executable(lab1_4_c src/4/c.c)

add_executable(lab1_5_a src/5/a.c)

add_executable(lab1_6_a src/6/a.c src/6/my_thread/my_thread.h src/6/my_thread/my_thread.c)

add_executable(lab1_7 src/7/main.c src/7/uthreadpool.c src/7/uthreadpool.h)
add_executable(lab1_7_v2 src/7/main.c src/7/uthreadpool2.c src/7/uthreadpool.h)
