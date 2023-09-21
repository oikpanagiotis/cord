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

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

/*
 * General purpose byte buffer object
 */
typedef struct cord_buffer_t {
    u8 *data;
    size_t size;
    size_t used;
} cord_buffer_t;

/*
 * Bump memory allocator
 *
 * Suitable for allocating a lot of small object (<4kb) that can be free'd
 * all at once. When a single block is full we create a new one pointed to
 * by next.
 */
typedef struct cord_bump_t {
    u8 *data;
    size_t capacity;
    size_t used;

    struct cord_bump_t *next;
} cord_bump_t;

cord_bump_t *cord_bump_create(void);
cord_bump_t *cord_bump_create_with_size(size_t size);
void cord_bump_destroy(cord_bump_t *bump);
void cord_bump_clear(cord_bump_t *bump);
void cord_bump_pop(cord_bump_t *bump, size_t size);
void *balloc(cord_bump_t *bump, size_t size);

/*
 * Bump memory allocator wrapper for allocating/freeing short-lived objects
 *
 * Used to mark the start and end of allocations that are required by a some
 * operation, so that we can pop the memory right after the operation finishes.
 * It's backed by a bump allocator that provides and owns the actual memory.
 */
typedef struct cord_temp_memory_t {
    size_t allocated;
    cord_bump_t *allocator;
    size_t block_idx;
    size_t block_off;
} cord_temp_memory_t;

cord_temp_memory_t cord_temp_memory_start(cord_bump_t *bump);
void cord_temp_memory_end(cord_temp_memory_t temp_memory);

#endif
