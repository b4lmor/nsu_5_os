#ifndef HASHMAP_H
#define HASHMAP_H

#define MAX_KEY_LENGTH 512
#define TABLE_SIZE 128

typedef struct node {
    char key[MAX_KEY_LENGTH];
    void *value;
    struct node *next;
} node_t;

typedef struct hashmap {
    node_t **table;
} hashmap_t;

hashmap_t *create_table();

void insert(hashmap_t *ht, const char *key, void *value);

void *get(hashmap_t *ht, const char *key);

void delete(const hashmap_t *ht, const char *key);

void destroy_hashmap(hashmap_t **ht);

#endif
