#ifndef HASHMAP_H
#define HASHMAP_H

#include "memory.h"

#include <stdbool.h>

#define MAX_HASHMAP_KEY_LEN 32

typedef void *(*cord_item_create_cb)(cord_bump_t *alloc);

typedef struct cord_hash_item_t {
    char key[MAX_HASHMAP_KEY_LEN];
    void *value;
    struct cord_hash_item_t *next;
    bool deleted;
} cord_hash_item_t;

/*
 * char* to void* hash map that uses linked lists to handle collisions
 *
 * Instead of the caller passing an already created object to the map,
 * in this case the hash map itself create an empty object based on
 * 'cord_item_create_cb' callback and returns it.
 *
 * The lifetime of the hashmap depends on the passed/underlying
 * memory allocator
 */
typedef struct cord_hashmap_t {
    cord_hash_item_t *items;
    size_t capacity;
    size_t num_items;
    cord_bump_t *allocator;
    cord_item_create_cb create_item_func;
} cord_hashmap_t;

/*
 * Create a new hash map by providing an empty object constructor function
 */
cord_hashmap_t *cord_hashmap_create(cord_bump_t *allocator,
                                    cord_item_create_cb create_item_func);

/*
 * Put a new key-value pair to the map and return the value's address
 *
 * In case of key-value pair allocation failure return NULL
 */
void *cord_hashmap_put(cord_hashmap_t *map, char *key);

/*
 * Retrieves the value of a key-value pair in the map
 *
 * Returns NULL if the value is not present in the map
 */
void *cord_hashmap_get(cord_hashmap_t *map, char *key);

#endif
