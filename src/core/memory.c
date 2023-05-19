#include "memory.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// cord_pool_t *cord_pool_create_with_size(size_t size) {
//     cord_pool_t *pool = malloc(sizeof(cord_pool_t));
//     if (!pool) {
//         return NULL;
//     }
//     pool->next = NULL;
//     pool->buffer.size = size;
//     pool->buffer.used = 0;
//     pool->buffer.data = calloc(1, size);
//     if (!pool->buffer.data) {
//         free(pool);
//         return NULL;
//     }

//     return pool;
// }

// cord_pool_t *cord_pool_create(void) {
//     const size_t default_size = KB(16);
//     return cord_pool_create_with_size(default_size);
// }

// void cord_pool_destroy(cord_pool_t *pool) {
//     if (pool) {
//         // FIXME: Iterate through the linked list and free every buffer
//         if (pool->buffer.data) {
//             free(pool->buffer.data);
//         }
//         free(pool);
//     }
// }

// void *palloc(cord_pool_t *pool, size_t size) {
//     if (pool->size < pool->offset + size) {
//         assert(1 == 0 && "Implement resize");
//         // TODO: Allocate new block
//         return NULL;
//     }

//     void *new_block = (void *)(pool->data + pool->offset);
//     pool->offset += size;
//     return new_block;
// }

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
    const size_t DEFAULT_SIZE = KB(128);
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

size_t cord_bump_remaining_memory(cord_bump_t *bump) {
    return bump->capacity - bump->used;
}

static bool is_power_of_two(uintptr_t x) {
	return (x & (x-1)) == 0;
}

/**
 * Aligned Memory:
 *  https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
 *  https://developer.ibm.com/technologies/systems/articles/pa-dalign/
*/
static uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;
    assert(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t)align;
    // Same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a - 1);

    if (modulo != 0) {
        // If 'p' address is not aligned, push the address to the
        // next value which is aligned
        p += a - modulo;
    }
    return p;
}

void *balloc(cord_bump_t *bump, size_t size) {
    if (size > cord_bump_remaining_memory(bump)) {
        // Out of memory
        return NULL;
    }

    uintptr_t curr = (uintptr_t)bump->data + (uintptr_t)bump->used;
    uintptr_t offset = align_forward(curr, 2 * sizeof(void *));
    offset -= (uintptr_t)bump->data;

    void *ptr = &bump->data[offset];
    bump->used += offset + size;
    return ptr;
}

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump) {
    return (cord_temp_memory_t){.allocator = bump, .allocated = 0};
}

void cord_temp_memory_end(cord_temp_memory_t temp_memory) {
    cord_bump_t *bump = temp_memory.allocator;
    cord_bump_pop(bump, temp_memory.allocated);
}
