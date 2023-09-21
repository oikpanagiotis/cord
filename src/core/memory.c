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
    cord_bump_t *it = bump;
    while (it->next) {
        it->used = 0;
        it = it->next;
    }
}

void cord_bump_pop(cord_bump_t *bump, size_t size) {
    size_t safe_size = (size > bump->used) ? bump->used : size;
    bump->used -= safe_size;
}

static cord_bump_t *find_last_block(cord_bump_t *bump) {
    cord_bump_t *last = bump;
    while (last->next) {
        last = last->next;
    }
    return last;
}

static size_t find_last_block_idx(cord_bump_t *bump) {
    size_t last_block_idx = 0;
    cord_bump_t *last = bump;
    while (last->next) {
        last_block_idx++;
        last = last->next;
    }
    return last_block_idx;
}

static size_t cord_bump_remaining_memory(cord_bump_t *bump) {
    // We can only allocate memory in the latest block
    cord_bump_t *last = find_last_block(bump);
    return last->capacity - last->used;
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

    cord_bump_t *last = find_last_block(bump);
    assert((last->used + size) < last->capacity);

    void *memory = &last->data[last->used];
    last->used += size;

    return memory;
}

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump) {
    size_t last_block_idx = find_last_block_idx(bump);
    cord_bump_t *last_block = find_last_block(bump);

    return (cord_temp_memory_t){
        .allocator = bump,
        .allocated = 0,
        .block_idx = last_block_idx,
        .block_off = last_block->used
    };
}

void cord_temp_memory_end(cord_temp_memory_t temp_memory) {
    size_t last_block_idx = find_last_block_idx(temp_memory.allocator);

    if (last_block_idx == temp_memory.block_idx) {
        cord_bump_pop(temp_memory.allocator, temp_memory.allocated);
    } else {
        // Find block of temp memory start
        cord_bump_t *temp_mem_block = temp_memory.allocator;
        for (size_t i = 0; i <= temp_memory.block_idx; i++) {
            temp_mem_block = temp_mem_block->next;
        }

        // Free only as much as we allocated in this temp context
        size_t max_pop = temp_mem_block->capacity - temp_memory.block_off;
        size_t pop_size = min(temp_memory.allocated, max_pop);
        cord_bump_pop(temp_mem_block, pop_size);

        cord_bump_t *it = temp_mem_block->next;
        while (it->next) {
            cord_bump_pop(it, it->capacity);
            it = it->next;
        }
    }
}

