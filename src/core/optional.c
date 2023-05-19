#include "optional.h"
#include "strings.h"
#include "typedefs.h"


cord_optional_t cord_optional_empty(cord_optional_value_type_t type) {
    return (cord_optional_t) { type, false, .val_ptr = NULL };
}

cord_optional_t cord_optional_of_i32(i32 value) {
   return (cord_optional_t) { OPTIONAL_VALUE_I32, true, .val_i32 = value };
}

cord_optional_t cord_optional_of_i64(i64 value) {
   return (cord_optional_t) { OPTIONAL_VALUE_I64, true, .val_i64 = value };
}

cord_optional_t cord_optional_of_u32(u32 value) {
   return (cord_optional_t) { OPTIONAL_VALUE_U32, true, .val_u32 = value };
}

cord_optional_t cord_optional_of_u64(u64 value) {
   return (cord_optional_t) { OPTIONAL_VALUE_U64, true, .val_u64 = value };
}

cord_optional_t cord_optional_of_f32(f32 value) {
   return (cord_optional_t) { OPTIONAL_VALUE_F32, true, .val_f32 = value };
}

cord_optional_t cord_optional_of_f64(f64 value) {
   return (cord_optional_t) { OPTIOANL_VALUE_F64, true, .val_f64 = value };
}

cord_optional_t cord_optional_of_bool(bool value) {
   return (cord_optional_t) { OPTIONAL_VALUE_BOOL, true, .val_bool = value};
}

cord_optional_t cord_optional_of_ptr(void *value) {
   return (cord_optional_t) { OPTIONAL_VALUE_PTR, true, .val_ptr = value };
}

bool cord_optional_has_value(cord_optional_t optional) {
    return optional.has_value; 
}

i32 cord_optional_get_i32(cord_optional_t optional) {
    return optional.val_i32;
}

i64 cord_optional_get_i64(cord_optional_t optional) {
    return optional.val_i64;
}

u32 cord_optional_get_u32(cord_optional_t optional) {
    return optional.val_u32;
}

u64 cord_optional_get_u64(cord_optional_t optional) {
    return optional.val_u64;
}

f32 cord_optional_get_f32(cord_optional_t optional) {
    return optional.val_f32;
}

f64 cord_optional_get_f64(cord_optional_t optional) {
    return optional.val_f64;
}

bool cord_optional_get_bool(cord_optional_t optional) {
    return optional.val_bool;
}

void *cord_optional_get_ptr(cord_optional_t optional) {
    return optional.val_ptr;
}
