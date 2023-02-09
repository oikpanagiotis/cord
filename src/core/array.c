#include "array.h"
#include "memory.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

cord_array_t *cord_array_create(cord_bump_t *allocator, size_t element_size) {
    assert(allocator);
    assert(element_size > 0);
    cord_array_t *array = balloc(allocator, sizeof(cord_array_t));
    if (!array) {
        return NULL;
    }

    array->element_size = element_size;
    array->num_elements = 0;
    array->allocator = allocator;

    const int default_capacity = 16;
    array->capacity = default_capacity;

    const size_t initial_array_size = element_size * (size_t)array->capacity;
    array->data = balloc(allocator, initial_array_size);
    assert(array->data);

    if (!array->data) {
        cord_bump_destroy(allocator);
        return NULL;
    }
    return array;
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

void *cord_array_get(cord_array_t *array, int index) {
    if (index < 0 || index > (int)array->capacity) {
        return NULL;
    }

    return array->data + array->element_size * index;
}

void cord_array_destroy(cord_array_t *array) {
    cord_bump_destroy(array->allocator);
}
