#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

#include "memory.h"


typedef struct cord_array_t {
    u8 *data;
    u32 capacity;
    u32 num_elements;
    size_t element_size;
    cord_pool_t *pool;
} cord_array_t;

cord_array_t *cord_array_create(cord_pool_t *pool, size_t element_size);
void *cord_array_push(cord_array_t *arr);
void *cord_array_get(cord_array_t *arr, int index);
void cord_array_destroy(cord_array_t *arr);

#endif