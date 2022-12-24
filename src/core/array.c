#include "array.h"

#include <stdlib.h>
#include <string.h>


cord_array_t *cord_array_create(cord_bump_t *allocator, size_t element_size) {
    cord_array_t *arr = balloc(allocator, sizeof(cord_array_t));
    if (!arr) {
        return NULL;
    }

    arr->element_size = element_size;
    arr->num_elements = 0;
    arr->allocator = allocator;

    const int default_capacity = 16;
    arr->capacity = default_capacity;

    arr->data = balloc(allocator, element_size * arr->capacity);
    if (!arr->data) {
        cord_bump_destroy(allocator);
        return NULL;
    }
    return arr;
}

void *cord_array_push(cord_array_t *array) {
    if (array->num_elements == array->capacity) {
        // When the array is full, we allocate a new one with double capacity
        u8 *current_array = array->data;
        u8 *new_array = balloc(array->allocator, array->capacity * 2);
        if (!new_array) {
            return NULL;
        }

        array->capacity *= 2;
        memcpy(new_array, current_array, array->num_elements * array->element_size);
        array->data = new_array;
    }

    void *new_element = array->data + (array->element_size * array->num_elements);
    array->num_elements++;
    return new_element;
}

void *cord_array_get(cord_array_t *arr, int index) {
    if (index < 0 || index > (int)arr->capacity) {
        return NULL;
    }

    return arr->data + arr->element_size * index;
}

void cord_array_destroy(cord_array_t *arr) {
    cord_bump_destroy(arr->allocator);
}
