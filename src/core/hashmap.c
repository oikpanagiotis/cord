#include "hashmap.h"
#include "memory.h"
#include "strings.h"

#include <assert.h>
#include <string.h>

static const cord_hash_item_t EMPTY_ITEM =
    (cord_hash_item_t){.key = "", .value = NULL, .next = NULL, .deleted = false};

static u64 hash(char *key) {
    u64 length = strlen(key);
    return length;
}

cord_hashmap_t *cord_hashmap_create(cord_bump_t *allocator,
                                    cord_item_create_cb create_item_func) {
    assert(allocator);
    assert(create_item_func);

    cord_hashmap_t *hashmap = balloc(allocator, sizeof(cord_hashmap_t));
    if (!hashmap) {
        return NULL;
    }

    const u32 default_capacity = 64;
    hashmap->items = balloc(allocator, default_capacity * sizeof(cord_hash_item_t));
    if (!hashmap->items) {
        return NULL;
    }
    for (size_t i = 0; i < default_capacity; i++) {
        hashmap->items[i] = EMPTY_ITEM;
    }

    hashmap->allocator = allocator;
    hashmap->create_item_func = create_item_func;
    hashmap->capacity = default_capacity;
    hashmap->num_items = 0;

    return hashmap;
}

static cord_hash_item_t *create_hash_item(cord_hashmap_t *map, char *key) {
    cord_hash_item_t *item = balloc(map->allocator, sizeof(cord_hash_item_t));
    if (!item) {
        return NULL;
    }
    strcpy(item->key, key);
    item->value = map->create_item_func(map->allocator);
    item->next = NULL;
    item->deleted = false;
    return item;
}

void *cord_hashmap_put(cord_hashmap_t *map, char *key) {
    assert(key);
    assert(strlen(key) <= MAX_HASHMAP_KEY_LEN);

    cord_hash_item_t *item = create_hash_item(map, key);
    if (!item) {
        return NULL;
    }

    size_t index = (size_t)hash(key) % map->capacity;
    assert((index >= 0) && (index < map->capacity));

    cord_hash_item_t *found_item = map->items + index;
    if (found_item == &EMPTY_ITEM) {
        map->items[index] = *item;
        map->num_items++;
        return item->value;
    }

    cord_hash_item_t *iter = NULL;
    for (iter = found_item; iter->next != NULL; iter = iter->next) {
        // decrement num items when marking items as deleted
        if (cstring_is_equal(iter->key, key)) {
            iter->deleted = true;
            map->num_items--;
        }
    }

    iter->next = item;
    map->num_items++;
    return item->value;
}

static cord_hash_item_t *find_item_in_list(cord_hash_item_t *head, char *key) {
    assert(head);
    assert(key);

    for (cord_hash_item_t *iter = head; iter != NULL; iter = iter->next) {
        if (!iter->deleted && cstring_is_equal(iter->key, key)) {
            return iter;
        }
    }

    return NULL;
}

void *cord_hashmap_get(cord_hashmap_t *map, char *key) {
    assert(key);
    assert(strlen(key) <= MAX_HASHMAP_KEY_LEN);

    size_t index = (size_t)hash(key) % map->capacity;
    assert(index >= 0 && index < map->capacity);

    cord_hash_item_t *head = map->items + index;
    if (head == &EMPTY_ITEM) {
        return NULL;
    }

    cord_hash_item_t *item = find_item_in_list(head, key);
    return item ? item->value : NULL;
}
