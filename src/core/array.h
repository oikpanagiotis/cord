#ifndef ARRAY_H
#define ARRAY_H

#include "memory.h"

#include <stdlib.h>
#include <string.h>



typedef struct cord_array_t {
    u8 *data;
    u32 capacity;
    u32 num_elements;
    size_t element_size;
    cord_bump_t *allocator;
} cord_array_t;

cord_array_t *cord_array_create(cord_bump_t *bump, size_t element_size);
void *cord_array_push(cord_array_t *arr);
void *cord_array_get(cord_array_t *arr, int index);
void cord_array_destroy(cord_array_t *arr);

#endif