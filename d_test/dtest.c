#include "dtest.h"
#include "dtypes.h"
#include "dutils.h"

#include <string.h>
#include <stdio.h>

#define PRINT_SUCCESS_TEST(message) (printf(GREEN message RESET "\n"))

static void print_outcome(bool success, void *left, void *right, usize size, PrintFn print_data)
{
    if (success)
    {
        PRINT_SUCCESS_TEST("OK");
        return;
    }

    // TODO: USE BINARY STRUCT YET TO BE CREATED TO PRINT OUT HEX CONTENT WITHIN ADDR
    if (print_data == NULL)
    {
        fprintf(stderr, RED "\nassertion `left == right` failed");
        fprintf(stderr, "left  : ");
        if (left == NULL)
            fprintf(stderr, "NULL\n");
        else
        {
            // PRINT HEX CONTENT
        }
        if (right == NULL)
            fprintf(stderr, "NULL\n");
        else
        {
            // PRINT HEX CONTENT
        }
        return;
    }

    fprintf(stderr, RED "assertion `left == right` failed\n" RESET);
    fprintf(stderr, "left  : ");
    if (left == NULL)
        fprintf(stderr, "NULL\n");
    else
    {
        print_data(left);
        fflush(stderr);
    }
    fprintf(stderr, "right : ");
    if (right == NULL)
        fprintf(stderr, "NULL\n");
    else
    {
        print_data(right);
        fflush(stderr);
    }
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

void d_assert_int(CHECK check_for, long long left, long long right)
{
    bool success = check_for == EQ ? left == right : left != right;

    print_outcome(success, &left, &right, sizeof(long long), print_int);
}

void d_assert_usize(CHECK check_for, usize left, usize right)
{
    bool success = check_for == EQ ? left == right : left != right;

    print_outcome(success, &left, &right, sizeof(usize), print_usize);
}

void d_assert_string(CHECK check_for, const char *left, const char *right)
{
    int cmp = strcmp(left, right);
    bool success = check_for == EQ ? cmp == 0 : cmp != 0;

    print_outcome(success, left, right, 0, print_string);
}

void d_assert_bool(CHECK check_for, bool left, bool right)
{
    bool success = check_for == EQ ? left == right : left != right;

    print_outcome(success, &left, &right, sizeof(bool), print_bool);
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
