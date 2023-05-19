#ifndef OPTIONAL_H
#define OPTIONAL_H

#include "typedefs.h"

#include <stdbool.h>

typedef enum cord_optional_value_type_t {
    OPTIONAL_VALUE_I32,
    OPTIONAL_VALUE_I64,
    OPTIONAL_VALUE_U32,
    OPTIONAL_VALUE_U64,
    OPTIONAL_VALUE_F32,
    OPTIOANL_VALUE_F64,
    OPTIONAL_VALUE_BOOL,
    OPTIONAL_VALUE_ARRAY, // See how we can make this work (array will always be a poitner
                          // to cord_array_t probably)
    OPTIONAL_VALUE_PTR
} cord_optional_value_type_t;

typedef struct cord_optional_t {
    cord_optional_value_type_t type;
    bool has_value;
    union {
        i32 val_i32;
        i64 val_i64;
        u32 val_u32;
        u64 val_u64;
        f32 val_f32;
        f64 val_f64;
        bool val_bool;
        void *val_ptr;
    };
} cord_optional_t;


bool cord_optional_has_value(cord_optional_t optional);

cord_optional_t cord_optional_empty(cord_optional_value_type_t type);

cord_optional_t cord_optional_of_i32(i32 value);
cord_optional_t cord_optional_of_i64(i64 value);
cord_optional_t cord_optional_of_u32(u32 value);
cord_optional_t cord_optional_of_u64(u64 value);
cord_optional_t cord_optional_of_f32(f32 value);
cord_optional_t cord_optional_of_f64(f64 value);
cord_optional_t cord_optional_of_bool(bool value);
cord_optional_t cord_optional_of_ptr(void *value);

i32 cord_optional_get_i32(cord_optional_t optional);
i64 cord_optional_get_i64(cord_optional_t optional);
u32 cord_optional_get_u32(cord_optional_t optional);
u64 cord_optional_get_u64(cord_optional_t optional);
f32 cord_optional_get_f32(cord_optional_t optional);
f64 cord_optional_get_f64(cord_optional_t optional);
bool cord_optional_get_bool(cord_optional_t optional);
void *cord_optional_get_ptr(cord_optional_t optional);

#endif