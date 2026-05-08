#ifndef D_TYPES_H
#define D_TYPES_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#define MAX_SIZE_T_VALUE SIZE_MAX

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef size_t usize;

typedef enum DResult
{
    D_OK = 0,
    D_ERR_ALLOC,
    D_ERR_INVALID_ARG,
    D_ERR_NOT_FOUND,
    D_ERR_NOT_EXIST,
    D_ERR_OVERFLOW
} DResult;

typedef enum
{
    D_COMPARE_ERROR = -2,
    D_COMPARE_EQUAL = 0,
    D_COMPARE_NOT_EQUAL = 1
} DCompareResult;

static inline char *d_types_print_result_as_str(DResult result)
{
    switch (result)
    {
    case D_OK:
        return "OK";
        break;
    case D_ERR_ALLOC:
        return "ERR_ALLOC";
    case D_ERR_INVALID_ARG:
        return "ERR_INVALID_ARG";
    case D_ERR_NOT_FOUND:
        return "ERR_NOT_FOUND";
    case D_ERR_NOT_EXIST:
        return "ERR_NOT_EXIST";
    case D_ERR_OVERFLOW:
        return "ERR_OVERFLOW";
    default:
        return "UNKNOWN_RESULT";
    }
}

#endif