#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>
#include <sys/types.h>

#define LIBRARY_NAME "cord"
#define LIBRARY_VERSION "0.1.0dev"
#define LIBRARY_REPOSITORY "github.com/oikpanagiotis/cord"

// Used as a read-only reference to a C string
typedef const char * string_ref;
typedef unsigned char byte;

typedef float f32;
typedef double f64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#endif