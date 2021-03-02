#ifndef ARRAY_H
#define ARRAY_H

typedef struct cord_array_t {
    void **items;
    int size;
    int capacity;
} cord_array_t;

cord_array_t *cord_array_create(void);
void cord_array_push(cord_array_t *arr, void *item);
void *array_get(cord_array_t *arr, int index);
void cord_array_free(cord_array_t *arr);

#endif