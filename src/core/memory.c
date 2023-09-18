#include "memory.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_SIZE KB(4)

cord_bump_t *cord_bump_create_with_size(size_t size) {
    assert(size > 0 && "size must be a positive integer");

    cord_bump_t *bump = malloc(sizeof(cord_bump_t));
    if (!bump) {
        return NULL;
    }

    bump->data = calloc(1, size);
    if (!bump->data) {
        free(bump);
        return NULL;
    }

    bump->capacity = size;
    bump->used = 0;
    bump->next = NULL;
    return bump;
}

cord_bump_t *cord_bump_create(void) {
    return cord_bump_create_with_size(DEFAULT_SIZE);
}

void cord_bump_destroy(cord_bump_t *bump) {
    if (bump) {
        if (bump->data) {
            free(bump->data);
        }
        free(bump);
    }
}

void cord_bump_clear(cord_bump_t *bump) {
    bump->used = 0;
}

void cord_bump_pop(cord_bump_t *bump, size_t size) {
    size_t safe_size = (size > bump->used) ? bump->used : size;
    bump->used -= safe_size;
}

static size_t cord_bump_remaining_memory(cord_bump_t *bump) {
    cord_bump_t *it = bump;
    // We can only allocate memory in the latest block
    while (it->next) {
        it = it->next;
    }
    return it->capacity - it->used;
}

void *balloc(cord_bump_t *bump, size_t size) {
    if (size > cord_bump_remaining_memory(bump)) {
        size_t bump_size = max(DEFAULT_SIZE, size);
        cord_bump_t *new_bump = cord_bump_create_with_size(bump_size);
        if (new_bump) {
            bump->next = new_bump;
        } else {
            return NULL;
        }
    }

    cord_bump_t *it = bump;
    while (it->next) {
        it = it->next;
    }

    assert((it->used + size) < it->capacity);

    void *memory = &it->data[it->used];
    it->used += size;

    return memory;
}

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump) {
    return (cord_temp_memory_t){.allocator = bump, .allocated = 0};
}

void cord_temp_memory_end(cord_temp_memory_t temp_memory) {
    cord_bump_t *bump = temp_memory.allocator;
    cord_bump_pop(bump, temp_memory.allocated);
}
