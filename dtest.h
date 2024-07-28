#ifndef __D_TEST_H__
#define __D_TEST_H__
#include <stdio.h>
#define MAX_VALUE_SIZE_T (~(size_t)0)

#define PRINT_SUCCESS_TEST(message) (printf(GREEN message RESET))

typedef char*(*DbgFn)(void*);

#define TEST(test_description, test_logic) do {\ 
                                                printf("%s\n", test_description);\ 
                                                test_logic \                                    
                                            } while(0)
#define d_assert(test_condition,left,right,pfn) do { \
    if (test_condition) { \
        PRINT_SUCCESS_TEST("OK"); \
    } else { \
        if (pfn != NULL) { \
            DbgFn fn = (DbgFn)pfn; \
            char *_left = fn(left); \
            char *_right = fn(right); \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
            free(_left); \
            free(_right); \
        } else { \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
        } \
    } \
} while (0)                                

#define assert_eq_custom(left, right, size, pfn) do { \
    if (memcmp(left, right, size) == 0) { \
        PRINT_SUCCESS_TEST("OK"); \
    } else { \
        if (pfn != NULL) { \
            DbgFn fn = (DbgFn)pfn; \
            char *_left = fn(left); \
            char *_right = fn(right); \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
            free(_left); \
            free(_right); \
        } else { \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
        } \
    } \
} while (0)

#define assert_ne_custom(left, right, size, pfn) do { \
    if (memcmp(left, right, size) != 0) { \
        PRINT_SUCCESS_TEST("OK"); \
    } else { \
        if (pfn != NULL) { \
            DbgFn fn = (DbgFn)pfn; \
            char *_left = fn(left); \
            char *_right = fn(right); \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
            free(_left); \
            free(_right); \
        } else { \
            fprintf(stderr, RED "assertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
        } \
    } \
} while (0)


#define assert_eq_null_custom(data, pfn) do { \
    if (data == NULL) { \
        PRINT_SUCCESS_TEST("OK"); \
    } else { \
        if (pfn != NULL) { \
            DbgFn fn = (DbgFn)pfn; \
            char *_data = fn(data); \
            fprintf(stderr, RED "assertion `left == right` failed\ndata: \"%s\"\n" RESET , data); \
            free(_left); \
            free(_right); \
        } else { \
            fprintf(stderr, RED "assertion `left == right` failed\ndata: \"%s\"" RESET, data); \
        } \
    } \
} while (0)

#define assert_ne_null_custom(data, pfn) do { \
    if (data != NULL) { \
        PRINT_SUCCESS_TEST("OK"); \
    } else { \
        if (pfn != NULL) { \
            DbgFn fn = (DbgFn)pfn; \
            char *_data = fn(data); \
            fprintf(stderr, RED "assertion `left == right` failed\ndata: \"%s\"\n" RESET , data); \
            free(_left); \
            free(_right); \
        } else { \
            fprintf(stderr, RED "assertion `left == right` failed\ndata: \"%s\"" RESET, data); \
        } \
    } \
} while (0)

#define d_assert_eq(left,right,size) assert_eq_custom(left,right,size,NULL)
#define assert_eq_null(data,pfn) assert_eq_null_custom(data,NULL)
#define d_assert_ne(left,right,size) assert_ne_custom(left,right,size,NULL)
#define assert_ne_null(data,pfn) assert_ne_null_custom(data,NULL)
#endif