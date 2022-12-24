#include "array.h"

#include <stdlib.h>
#include <string.h>

cord_array_t *cord_array_create(cord_pool_t *pool, size_t element_size) {
    cord_array_t *arr = palloc(pool, sizeof(cord_array_t));
    if (!arr) {
        return NULL;
    }

    arr->element_size = element_size;
    arr->num_elements = 0;
    arr->pool = pool;

    const int default_capacity = 16;
    arr->capacity = default_capacity;

    arr->data = palloc(pool, element_size * arr->capacity);
    if (!arr->data) {
        cord_pool_destroy(pool);
        return NULL;
    }
    return arr;
}

void *cord_array_push(cord_array_t *arr) {
    if (arr->num_elements == arr->capacity) {
        // When the array is full, we allocate a new one with double capacity
        u8 *current_array = arr->data;
        u8 *new_array = palloc(arr->pool, arr->capacity * 2);
        if (!new_array) {
            return NULL;
        }

        arr->capacity *= 2;
        memcpy(new_array, current_array, arr->num_elements * arr->element_size);
        arr->data = new_array;
    }

    void *new_element = arr->data + (arr->element_size * arr->num_elements);
    arr->num_elements++;
    return new_element;
}

void *cord_array_get(cord_array_t *arr, int index) {
    if (index < 0 || index > (int)arr->capacity) {
        return NULL;
    }

    return arr->data + arr->element_size * index;
}

void cord_array_destroy(cord_array_t *arr) {
    cord_pool_destroy(arr->pool);
}
