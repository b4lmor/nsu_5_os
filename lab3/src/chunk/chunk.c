#include "../../include/chunk.h"

#include <stdio.h>
#include <string.h>

chunk_container_t *create_chunk_container() {
    chunk_container_t *container = malloc(sizeof(chunk_container_t));
    if (!container) {
        return NULL;
    }
    container->first = NULL;
    container->last = NULL;
    container->count = 0;
    return container;
}

void __add_chunk(chunk_container_t *container, const void *data, const size_t element_size, const size_t size) {
    if (!container || size == 0 || !data) {
        return;
    }
    chunk_t *new_chunk = malloc(sizeof(chunk_t));
    if (!new_chunk) {
        perror("Failed to create chunk");
        return;
    }
    new_chunk->index = container->last ? container->last->index + 1 : 0;
    new_chunk->element_size = element_size;
    new_chunk->size = size;
    new_chunk->next = NULL;
    new_chunk->prev = NULL;
    new_chunk->data = malloc(element_size * size);
    if (!new_chunk->data) {
        perror("Failed to allocate data chunk");
        free(new_chunk);
        return;
    }
    memcpy(new_chunk->data, data, element_size * size);
    if (container->last) {
        container->last->next = new_chunk;
    } else {
        container->first = new_chunk;
    }
    new_chunk->prev = container->last;
    container->last = new_chunk;
    container->count++;
}

void free_chunk_container(chunk_container_t **container) {
    if (!container) return;
    chunk_t *current = (*container)->first;
    while (current) {
        chunk_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(*container);
    *container = NULL;
}

chunk_t *get_chunk(chunk_container_t *container, const size_t index) {
    if (!container || index >= container->count) {
        return NULL;
    }
    const char forward = index < container->count / 2;
    chunk_t *current = forward ? container->first : container->last;
    while (current->index != index) {
        current = forward ? current->next : current->prev;
    }
    return current;
}
