#include <stdbool.h>
#include "d_types.h"

#define TEST(test, name)

typedef void (*PrintFn)(void *);

typedef enum CHECK
{
    EQ,
    NEQ,
} CHECK;

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

void d_assert(CHECK check_for, void *left, void *right, usize size, PrintFn print_function);
void d_assert_int(CHECK check_for, long long left, long long right);
void d_assert_usize(CHECK check_for, usize left, usize right);
void d_assert_string(CHECK check_for, const char *left, const char *right);
void d_assert_bool(CHECK check_for, bool left, bool right);

#define ASSERT_EQ_CUSTOM(left, right, size, print_function) \
    do                                                      \
    {                                                       \
        d_assert(EQ, left, right, size, print_function)     \
    } while (0)

/**/

#define ASSERT_EQ_STRING(left, right)    \
    do                                   \
    {                                    \
        d_assert_string(EQ, left, right) \
    } while (0)

#define ASSERT_EQ_BOOL(left, right)    \
    do                                 \
    {                                  \
        d_assert_bool(EQ, left, right) \
    } while (0)

#define ASSERT_EQ_USIZE(left, right)    \
    do                                  \
    {                                   \
        d_assert_usize(EQ, left, right) \
    } while (0)

#define ASSERT_EQ_INT(left, right)    \
    do                                \
    {                                 \
        d_assert_int(EQ, left, right) \
    } while (0)

/**/

/**/

#define ASSERT_NEQ_STRING(left, right)    \
    do                                    \
    {                                     \
        d_assert_string(NEQ, left, right) \
    } while (0)

#define ASSERT_NEQ_BOOL(left, right)    \
    do                                  \
    {                                   \
        d_assert_bool(NEQ, left, right) \
    } while (0)

#define ASSERT_NEQ_USIZE(left, right)    \
    do                                   \
    {                                    \
        d_assert_usize(NEQ, left, right) \
    } while (0)

#define ASSERT_NEQ_INT(left, right)    \
    do                                 \
    {                                  \
        d_assert_int(NEQ, left, right) \
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
