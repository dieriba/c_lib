#ifndef D_TEST_H
#define D_TEST_H

#include <stdbool.h>
#include "d_types.h"

typedef void (*TestFn)(void);

typedef struct DTest
{
    const char *test_name;
    TestFn test_fn;
} DTest;

void d_test_impl(bool eval, const char *test, const char *file, int line);
void d_test_run_tests_impl(DTest tests[], usize nb_test);

#define D_TEST_EXPR(expr)                             \
    do                                                \
    {                                                 \
        d_test_impl(expr, #expr, __FILE__, __LINE__); \
    } while (0)
#define D_TEST_NOT_NULL(val)                                           \
    do                                                                 \
    {                                                                  \
        d_test_impl(val != NULL, #val " != NULL", __FILE__, __LINE__); \
    } while (0)
#define D_TEST_NULL(val)                                               \
    do                                                                 \
    {                                                                  \
        d_test_impl(val == NULL, #val " == NULL", __FILE__, __LINE__); \
    } while (0)

#define D_TEST_STR_EQ(s1, s2)                                             \
    do                                                                    \
    {                                                                     \
        d_test_impl(!strcmp(s1, s2), #s1 " == " #s2, __FILE__, __LINE__); \
    } while (0)

#define D_TEST_STR_NEQ(s1, s2)                                           \
    do                                                                   \
    {                                                                    \
        d_test_impl(strcmp(s1, s2), #s1 " != " #s2, __FILE__, __LINE__); \
    } while (0)

#define D_TEST_MEM_EQ(mem1, mem2, size)                                                 \
    do                                                                                  \
    {                                                                                   \
        d_test_impl(!memcmp(mem1, mem2, size), #mem1 " == " #mem2, __FILE__, __LINE__); \
    } while (0)

#define D_TEST_MEM_NEQ(mem1, mem2, size)                                               \
    do                                                                                 \
    {                                                                                  \
        d_test_impl(memcmp(mem1, mem2, size), #mem1 " != " #mem2, __FILE__, __LINE__); \
    } while (0)

#define D_TEST_GENERATE_TEST(test_function) \
    (DTest) { .test_fn = test_function, .test_name = #test_function }

#define D_TEST_RUN_TESTS(tests) \
    d_test_run_tests_impl(tests, sizeof(tests) / sizeof(DTest))

#endif