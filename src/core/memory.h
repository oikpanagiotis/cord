#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "typedefs.h"

#define array_length(arr) (sizeof(arr) / sizeof(*arr))

#define Bytes(n) ((size_t)n)
#define KB(n) (size_t)(n * 1024)
#define MB(n) (size_t)(n * 1024 * 1024)
#define GB(n) (size_t)(n * 1024 * 1024 * 1024)

/*
 * General purpose byte buffer object
 * Used as a building block for allocators or other structures
 */
typedef struct cord_buffer_t {
    u8 *data;
    size_t size;
    size_t used;
} cord_buffer_t;

typedef struct cord_memory_entry_t {
    uintptr_t *base_address;
    size_t end_of_entry_index;
    u32 pool_index;

    struct cord_memory_entry_t *next;
} cord_memory_entry_t;

/*
 * Memory pool allocator
 *
 * Suitable for allocating a lot of objects of the same type.
 * The memory is allocated in blocks of a fixed size and when we
 * request more memory than available, the allocator creates a new
 * pool.
 *
 * Pool allocator allows us to free objects that were allocated
 * in the middle of the allocator's lifetime.
 */
typedef struct cord_pool_t {
    cord_buffer_t internal;
    cord_buffer_t buffer;
    cord_buffer_t *next;

    // Points to unoccupied pools so we can reuse them.
    cord_memory_entry_t free_list;
} cord_pool_t;

// cord_pool_t *cord_pool_create(void);
// cord_pool_t *cord_pool_create_with_size(size_t size);
// void cord_pool_destroy(cord_pool_t *pool);

// void *palloc(cord_pool_t *pool, size_t size);
// void pfree(cord_pool_t *pool);


// TODO : TOTAL REFACTOR OF ALLOCATIONS / DEALLOCATIONS
// Sharing the same allocator is too complicated to do right
// SOLUTION 1:
// Each large object should have it's own cord_bump_t allocator and containers will store pointers
// SOLUTION 2:
// Containers, Builders and any large object that is used to work with an entity and ISN't an entity itself
// should have it's own allocator. Memory for entities will be allocated by other data structures
// so we will need to be able to do resizing which means the apis will need to accept constructors
// and destructors for the entities we are going to use
// (NOTE: destructors might not be mandatory since when resizing we will need the constructor to replicate
// the entity and we are going to free all of the old-container-memory at once)


/*
 * Bump style allocator
 *
 * Suitable for allocating a lot of object of variable size that can be free'd
 * at once. The underlying memory is a linear fixed size buffer
 */
typedef struct cord_bump_t {
    u8 *data;
    size_t capacity;
    size_t used;
} cord_bump_t;

cord_bump_t *cord_bump_create(void);
cord_bump_t *cord_bump_create_with_size(size_t size);
void cord_bump_destroy(cord_bump_t *bump);
void cord_bump_clear(cord_bump_t *bump);
void cord_bump_pop(cord_bump_t *bump, size_t size);
size_t cord_bump_remaining_memory(cord_bump_t *bump);
void *balloc(cord_bump_t *bump, size_t size);

/*
 * Bump memory allocator wrapper for allocating/freeing short-lived objects
 *
 * Used to mark the start and end memory index of an allocator so after the
 * operations are performed the temp memory allocated can be poped like in
 * a stack. It's backed by a bump allocator that provides and owns the actual
 * memory.
 *
 */
typedef struct cord_temp_memory_t {
    size_t allocated;
    cord_bump_t *allocator;
} cord_temp_memory_t;

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump);
void cord_temp_memory_end(cord_temp_memory_t temp_memory);

#endif