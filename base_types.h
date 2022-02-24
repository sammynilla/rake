
#pragma once

#define global static
#define internal static
#define local_persist static

#include <stdint.h>

typedef int8_t    i8;   /* char */
typedef int16_t   i16;  /* short */
typedef int32_t   i32;  /* int, long */
typedef int64_t   i64;  /* long long */

typedef uint8_t   u8;   /* unsigned char */
typedef uint16_t  u16;  /* unsigned short */
typedef uint32_t  u32;  /* unsigned int, unsigned long */
typedef uint64_t  u64;  /* unsigned long long */

typedef i32       b32;
typedef float     r32;
typedef double    r64;

#define M_PI 3.14159265358979323846264338327950288
