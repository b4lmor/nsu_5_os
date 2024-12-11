cmake_minimum_required(VERSION 3.29)
project(cache_proxy C)

set(CMAKE_C_STANDARD 11)

find_package(CURL REQUIRED)

add_executable(c-proxy
        src/main.c
        include/common.h
        src/common/common.c
        include/context.h
        src/context/context.c
        include/http_utils.h
        src/http_utils/http_utils.c
        include/server.h
        src/server/server.c
        include/handler.h
        src/handler/handler.c
        include/chunk.h
        src/chunk/chunk.c
        include/subscription.h
        src/chunk/subscription.c
        include/hashmap.h
        src/common/hashmap.c
        include/log.h
        src/log/log.c
        include/cache.h
        src/chunk/cache.c
        include/threadpool.h
        src/threadpool/threadpool.c
)
target_link_libraries(c-proxy ${CURL_LIBRARIES})
target_include_directories(c-proxy PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(c-proxy PRIVATE PARALLEL)
