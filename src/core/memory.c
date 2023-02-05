#include "memory.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

cord_pool_t *cord_pool_create_with_size(size_t size) {
    cord_pool_t *pool = malloc(sizeof(cord_pool_t));
    if (!pool) {
        return NULL;
    }
    pool->size = size;
    pool->offset = 0;
    pool->next = NULL;
    pool->data = calloc(1, size);
    if (!pool->data) {
        free(pool);
        return NULL;
    }

    return pool;
}

cord_pool_t *cord_pool_create(void) {
    const size_t default_size = KB(16);
    return cord_pool_create_with_size(default_size);
}

void cord_pool_destroy(cord_pool_t *pool) {
    if (pool) {
        if (pool->data) {
            free(pool->data);
        }
        free(pool);
    }
}

void *palloc(cord_pool_t *pool, size_t size) {
    if (pool->size < pool->offset + size) {
        assert(1 == 0 && "Implement resize");
        // TODO: Allocate new block
        return NULL;
    }

    void *new_block = (void *)(pool->data + pool->offset);
    pool->offset += size;
    return new_block;
}

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
    return bump;
}

cord_bump_t *cord_bump_create(void) {
    const size_t INITIAL_SIZE = 512;
    return cord_bump_create_with_size(INITIAL_SIZE);
}

void cord_bump_destroy(cord_bump_t *bump) {
    if (bump) {
        if (bump->data) {
            free(bump->data);
        }
        free(bump);
    }
}

void cord_bump_clear(cord_bump_t *bump) { bump->used = 0; }

void cord_bump_pop(cord_bump_t *bump, size_t size) {
    size_t safe_size = (size > bump->used) ? bump->used : size;
    bump->used -= safe_size;
}

size_t cord_bump_remaining_memory(cord_bump_t *bump) {
    return bump->capacity - bump->used;
}

void *balloc(cord_bump_t *bump, size_t size) {
    if (size > cord_bump_remaining_memory(bump)) {
        void *reallocated = realloc(bump->data, bump->capacity * 2);
        if (!reallocated) {
            return NULL;
        }
        bump->data = reallocated;
        bump->capacity *= 2;
    }

    void *block = bump->data + bump->used;
    bump->used += size;
    return block;
}

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump) {
    return (cord_temp_memory_t){.allocator = bump, .allocated = 0};
}

void cord_temp_memory_end(cord_temp_memory_t temp_memory) {
    cord_bump_t *bump = temp_memory.allocator;
    cord_bump_pop(bump, temp_memory.allocated);
}
