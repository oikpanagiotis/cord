#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdlib.h>

#include "typedefs.h"

#define KB(n) (n * 1024)
#define MB(n) (n * 1024 * 1024)
#define GB(n) (n * 1024 * 1024 * 1024)

/*
*   Memory pool allocator
*
*   Suitable for allocating a lot of objects of the same type.
*   The memory is allocated in blocks of a fixed size and when we
*   request more memory than available, the allocator creates a new
*   pool.
*
*   Pool allocator allows us to free objects that were allocated
*   in the middle of the allocator's lifetime.
*/
typedef struct cord_pool_t {
    u8 *data;
    size_t offset;
    size_t size;
    struct cord_pool_t *next;

    // Points to unoccupied pools so we can reuse them.
    struct cord_pool_t *free_list;
} cord_pool_t;

cord_pool_t *cord_pool_create(void);
cord_pool_t *cord_pool_create_with_size(size_t size);
void cord_pool_destroy(cord_pool_t *pool);

void *palloc(cord_pool_t *pool, size_t size);

/*
*   Bump style allocator
*
*   Suitable for allocating a lot of object of the same type.
*   The underlying memory is allocated linearly and can grow indefinetly
*   as long as there is enough available memory. 
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
size_t cord_bump_remaining_memory(cord_bump_t *bump);
void *balloc(cord_bump_t *bump, size_t size);


#endif