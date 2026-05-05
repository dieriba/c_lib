#include <string.h>
#include <stdio.h>

#include "d_test.h"
#include "d_types.h"
#include "d_utils.h"

typedef enum OPERANDS
{
    _NULL,
    _NOT_NULL
} OPERANDS;

typedef enum PRINT_TYPE
{
    BOOL,
    USIZE,
    INT,
    STRING
} PRINT_TYPE;

#define PRINT_SUCCESS_TEST(message) (printf(GREEN message RESET "\n"))

static void print_outcome(bool success, const char *format, ...)
{
}

void d_assert(CHECK check_for, void *left, void *right, usize size, PrintFn print_function)
{
    bool success;

    if (left == NULL || right == NULL)
    {
        success = check_for == EQ ? left == right : left != right;
    }
    else
    {
        int cmp_result = memcmp(left, right, size);
        success = check_for == EQ ? cmp_result == 0 : cmp_result != 0;
    }

    print_outcome(success, left, right, size, print_function);
}

static void print_bool(void *b)
{
    bool success = *(bool *)b;
    char *success_msg;

    if (success)
        success_msg = "true";
    else
        success_msg = "false";

    fprintf(stderr, "%s\n", success_msg);
}

static void print_string(void *str)
{
    fprintf(stderr, "%s\n", (char *)str);
}

static void print_usize(void *ptr)
{
    usize number = *(usize *)ptr;
    fprintf(stderr, "%lu\n", number);
}

static void print_int(void *ptr)
{
    long long number = *(long long *)ptr;
    fprintf(stderr, "%lli\n", number);
}

void d_assert_int_equal(intmax_t left, intmax_t right, const char *const file, const int line)
{
    bool success = left == right;
}

void d_assert_usize(CHECK check_for, uintmax_t left, uintmax_t right, const char *const file, const int line)
{
    bool success = check_for == EQ ? left == right : left != right;

    print_outcome(success, &left, &right, sizeof(usize), print_usize);
}

void d_assert_string(CHECK check_for, const char *left, const char *right, const char *const file, const int line)
{
    int cmp = strcmp(left, right);
    bool success = check_for == EQ ? cmp == 0 : cmp != 0;

    print_outcome(success, left, right, 0, print_string);
}

void d_assert_bool(CHECK check_for, bool left, bool right, const char *const file, const int line)
{
    bool success = check_for == EQ ? left == right : left != right;

    print_outcome(success, &left, &right, sizeof(bool), print_bool);
}
