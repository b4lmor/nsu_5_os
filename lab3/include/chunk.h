#ifndef CHUNK_C
#define CHUNK_C

#include <stdlib.h>

typedef struct chunk {
    char is_last;
    size_t index;
    size_t element_size;
    size_t size;
    struct chunk *next;
    struct chunk *prev;
    void *data;
} chunk_t;

typedef struct chunk_container {
    chunk_t *first;
    chunk_t *last;
    size_t count;
} chunk_container_t;

chunk_container_t* create_chunk_container();

void __add_chunk(chunk_container_t *container, const void *data, size_t element_size, size_t size);

void free_chunk_container(chunk_container_t **container);

chunk_t* get_chunk(chunk_container_t *container, size_t index);

#endif
