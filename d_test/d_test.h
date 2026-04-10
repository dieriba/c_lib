#include <stdbool.h>

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

/* Function prototype for test functions. */
typedef void (*DTestFunction)();

struct DUnitTest {
    const char *name;
    DTestFunction test_func;
};

void d_assert(CHECK check_for, void *left, void *right, usize size, PrintFn print_function);
void d_assert_int(CHECK check_for, long long left, long long right);
void d_assert_usize(CHECK check_for, usize left, usize right);
void d_assert_string(CHECK check_for, const char *left, const char *right);
void d_assert_bool(CHECK check_for, bool left, bool right);

#define assert_eq_custom(left, right, size, print_function) \
    do                                                      \
    {                                                       \
        d_assert(EQ, left, right, size, print_function)     \
    } while (0)

/**/

#define assert_string_eq(left, right)    \
    do                                   \
    {                                    \
        d_assert_string(EQ, left, right) \
    } while (0)

#define assert_bool_eq(left, right)    \
    do                                 \
    {                                  \
        d_assert_bool(EQ, left, right) \
    } while (0)

#define assert_usize_eq(left, right)    \
    do                                  \
    {                                   \
        d_assert_usize(EQ, left, right) \
    } while (0)

#define assert_int_eq(left, right)    \
    do                                \
    {                                 \
        d_assert_int(EQ, left, right) \
    } while (0)

/**/

/**/

#define assert_string_not_eq(left, right)    \
    do                                    \
    {                                     \
        d_assert_string(NEQ, left, right) \
    } while (0)

#define assert_bool_not_eq(left, right)    \
    do                                  \
    {                                   \
        d_assert_bool(NEQ, left, right) \
    } while (0)

#define assert_usize_not_eq(left, right)    \
    do                                   \
    {                                    \
        d_assert_usize(NEQ, left, right) \
    } while (0)

#define asset_int_not_eq(left, right)    \
    do                                 \
    {                                  \
        d_assert_int(NEQ, left, right) \
    } while (0)

/**/

/**/

#define assert_null(left)                 \
    do                                    \
    {                                     \
        d_assert(EQ, left, NULL, 0, NULL) \
    } while (0)

#define assert_not_null(left)              \
    do                                     \
    {                                      \
        d_assert(NEQ, left, NULL, 0, NULL) \
    } while (0)


#define assert(expr)


/**/
