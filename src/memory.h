#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdlib.h>

#define KB(n) (n * 1024)
#define MB(n) (n * 1024 * 1024)
#define GB(n) (n * 1024 * 1024 * 1024)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct cord_pool_t {
    u8 *data;
    size_t offset;
    size_t size;
    struct cord_pool_t *next;
} cord_pool_t;

cord_pool_t *cord_pool_create(void);
cord_pool_t *cord_pool_create_with_size(size_t size);
void cord_pool_destroy(cord_pool_t *pool);

void *alloc(cord_pool_t *pool, size_t size);

#endif