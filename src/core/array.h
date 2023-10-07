#ifndef ARRAY_H
#define ARRAY_H

#include "memory.h"

#include <stdlib.h>
#include <string.h>

/*
 * Dynamic array implementation that can be used to store
 * POD type
 *
 * It's backed by a bump allocator and will request twice
 * the capacity in size when it runs out of memory. There
 * is no destroy function since we need to destroy the bump
 * allocator to get rid of the memory altogether.
 */
typedef struct cord_array_t {
    u8 *data;
    size_t capacity;
    size_t num_elements;
    size_t element_size;
    cord_bump_t *allocator;
} cord_array_t;

cord_array_t *cord_array_create(cord_bump_t *bump, size_t element_size);
void *cord_array_push(cord_array_t *array);
void *cord_array_get(cord_array_t *array, int index);

#endif
