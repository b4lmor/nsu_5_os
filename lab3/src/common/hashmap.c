#include "../../include/hashmap.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>

hashmap_t *create_table() {
    hashmap_t *ht = malloc(sizeof(hashmap_t));
    if (ht == NULL) {
        perror("Failed to allocate memory for hashmap_t");
        return NULL;
    }
    ht->table = malloc(sizeof(node_t *) * TABLE_SIZE);
    if (ht->table == NULL) {
        perror("Failed to allocate memory for hashmap_t");
        return NULL;
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        ht->table[i] = NULL;
    }
    return ht;
}

void insert(hashmap_t *ht, const char *key, void *value) {
    const unsigned int index = hashn(key, MAX_KEY_LENGTH) % TABLE_SIZE;
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL) {
        perror("Failed to allocate memory for node_t");
        return;
    }
    strncpy(new_node->key, key, MAX_KEY_LENGTH);
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
}

void *get(hashmap_t *ht, const char *key) {
    const unsigned int index = hashn(key, MAX_KEY_LENGTH) % TABLE_SIZE;
    const node_t *current = ht->table[index];
    while (current != NULL) {
        if (strncmp(current->key, key, MAX_KEY_LENGTH) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

void delete(const hashmap_t *ht, const char *key) {
    const unsigned int index = hashn(key, MAX_KEY_LENGTH) % TABLE_SIZE;
    node_t *current = ht->table[index];
    node_t *previous = NULL;
    while (current != NULL) {
        if (strncmp(current->key, key, MAX_KEY_LENGTH) == 0) {
            if (previous == NULL) {
                ht->table[index] = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void destroy_hashmap(hashmap_t **ht) {
    if (ht == NULL || *ht == NULL) {
        return;
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        node_t *current = (*ht)->table[i];
        while (current != NULL) {
            node_t *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free((*ht)->table);
    free(*ht);
    *ht = NULL;
}
