#ifndef D_TEST_H
#define D_TEST_H

#include <stdbool.h>
#include "d_types.h"

#define TEST(test, name)

typedef void (*PrintFn)(void *);

/* Function prototype for test functions. */
typedef void (*DUnitTestFunction)(void);

typedef struct DUnitTest
{
    const char *name;
    DUnitTestFunction test_func;
} DUnitTest;

typedef struct DGroupTest
{
    const DUnitTest *tests;
    const usize number_of_tests;
} DGroupTest;

typedef enum CHECK
{
    EQ,
    NEQ,
} CHECK;

#define d_setup_run_tests(tests)

void d_assert(CHECK check_for, void *left, void *right, usize size, PrintFn print_function);
void d_assert_int(CHECK check_for, intmax_t left, intmax_t right, const char *const file, const int line);
void d_assert_usize(CHECK check_for, uintmax_t left, uintmax_t right, const char *const file, const int line);
void d_assert_string(CHECK check_for, const char *left, const char *right, const char *const file, const int line);
void d_assert_bool(CHECK check_for, bool left, bool right, const char *const file, const int line);

#define ASSERT_EQ_CUSTOM(left, right, size, print_function) \
    do                                                      \
    {                                                       \
        d_assert(EQ, left, right, size, print_function)     \
    } while (0)

/**/

#define ASSERT_EQ_STRING(left, right)                        \
    do                                                       \
    {                                                        \
        d_assert_string(EQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_EQ_BOOL(left, right)                        \
    do                                                     \
    {                                                      \
        d_assert_bool(EQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_EQ_USIZE(left, right)                        \
    do                                                      \
    {                                                       \
        d_assert_usize(EQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_EQ_INT(left, right)                        \
    do                                                    \
    {                                                     \
        d_assert_int(EQ, left, right, __FILE__, __LINE__) \
    } while (0)

/**/

/**/

#define ASSERT_NEQ_STRING(left, right)                        \
    do                                                        \
    {                                                         \
        d_assert_string(NEQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_NEQ_BOOL(left, right)                        \
    do                                                      \
    {                                                       \
        d_assert_bool(NEQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_NEQ_USIZE(left, right)                        \
    do                                                       \
    {                                                        \
        d_assert_usize(NEQ, left, right, __FILE__, __LINE__) \
    } while (0)

#define ASSERT_NEQ_INT(left, right)                        \
    do                                                     \
    {                                                      \
        d_assert_int(NEQ, left, right, __FILE__, __LINE__) \
    } while (0)

/**/

/**/

#define ASSERT_NULL(left)                 \
    do                                    \
    {                                     \
        d_assert(EQ, left, NULL, 0, NULL) \
    } while (0)

#define ASSERT_NOT_NULL(left)              \
    do                                     \
    {                                      \
        d_assert(NEQ, left, NULL, 0, NULL) \
    } while (0)

/**/
#endif