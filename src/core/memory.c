#include "memory.h"
#include "log.h"

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SIZE KB(4)

typedef struct block_data_t {
    cord_bump_t *block;
    size_t idx;
} block_data_t;

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

static block_data_t find_last_block(cord_bump_t *bump) {
    size_t last_block_idx = 0;
    cord_bump_t *last = bump;
    while (last->next) {
        last_block_idx++;
        last = last->next;
    }
    return (block_data_t){.block = last, .idx = last_block_idx};
}

static size_t cord_bump_remaining_memory(cord_bump_t *bump) {
    // We can only allocate memory in the latest block
    block_data_t last_bdata = find_last_block(bump);
    cord_bump_t *last = last_bdata.block;

    return last->capacity - last->used;
}

void *balloc(cord_bump_t *bump, size_t size) {
    size_t alignment = alignof(max_align_t);
    size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);

    if (aligned_size > cord_bump_remaining_memory(bump)) {
        size_t bump_size = max(DEFAULT_SIZE, aligned_size);
        cord_bump_t *new_bump = cord_bump_create_with_size(bump_size);
        if (new_bump) {
            bump->next = new_bump;
        } else {
            return NULL;
        }
    }

    block_data_t last_bdata = find_last_block(bump);
    cord_bump_t *last = last_bdata.block;
    assert((last->used + size) < last->capacity);

    void *memory = &last->data[last->used];
    memset(memory, 0, aligned_size);
    last->used += aligned_size;

    return memory;
}

void *cord_bump_index(cord_bump_t *bump, size_t index) {
    size_t alignment = alignof(max_align_t);
    size_t aligned_size = (index + alignment - 1) & ~(alignment - 1);
    return bump->data + aligned_size;
}

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump) {
    block_data_t last_bdata = find_last_block(bump);

    return (cord_temp_memory_t){.allocator = bump,
                                .allocated = 0,
                                .block_idx = last_bdata.idx,
                                .block_off = last_bdata.block->used};
}

void cord_temp_memory_end(cord_temp_memory_t temp_memory) {
    block_data_t last_bdata = find_last_block(temp_memory.allocator);

    if (last_bdata.idx == temp_memory.block_idx) {
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
