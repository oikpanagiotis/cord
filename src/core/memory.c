#include "memory.h"

#include <assert.h>
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

cord_bump_t *cord_bump_create(void) {

}

cord_bump_t *cord_bump_create_with_size(size_t size) {

}

void cord_bump_destroy(cord_bump_t *bump) {

}

void *balloc(cord_bump_t *bump, size_t size) {
    return NULL;
}