#ifndef __D_TYPES_H
#define __D_TYPES_H

#include <sys/types.h>
#include <stdbool.h>
#define MAX_SIZE_T_VALUE (~(size_t)0)

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef size_t usize;
#endif