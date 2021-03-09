#include "array.h"

#include <stdlib.h>

cord_array_t *cord_array_create(void) {
    cord_array_t *arr = malloc(sizeof(cord_array_t));
    if (!arr) {
        return NULL;
    }

    arr->size = 0;
    arr->capacity = 16;
    arr->items = malloc(arr->capacity * sizeof(void *));
    if (!arr->items) {
        free(arr);
        return NULL;
    }
    return arr;
}

void cord_array_push(cord_array_t *arr, void *item) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->items = realloc(arr->items, arr->capacity * sizeof(void *));
    }
    arr->items[arr->size++] = item;
}

void *cord_array_get(cord_array_t *arr, int index) {
    if (index > arr->capacity - 1) {
        return NULL;
    }

    return arr->items[index];
}

void cord_array_free(cord_array_t *arr) {
    if (arr) {
        free(arr->items);
        free(arr);
    }
}
