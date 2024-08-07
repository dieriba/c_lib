#ifndef __D_TEST_H__
#define __D_TEST_H__
#include <stdio.h>
#include <dutils.h>
#include <sys/wait.h>
#define MAX_VALUE_SIZE_T (~(size_t)0)

#define PRINT_SUCCESS_TEST(message) (printf(GREEN message RESET))

typedef char*(*DbgFn)(void*);

#define TEST(test_description,test_logic) do { \
    printf("\n%s\n", test_description); \
    test_logic \
} while(0)

#define d_assert(test_condition,left,right,pfn) do { \
    int pid = fork(); \
    if (pid == 0) { \
        if (test_condition) { \
            PRINT_SUCCESS_TEST("OK "); \
        } else { \
            if (pfn != NULL) { \
                DbgFn fn = (DbgFn)pfn; \
                char *_left = fn(left); \
                char *_right = fn(right); \
                fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
                free(_left); \
                free(_right); \
            } else { \
                fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
            } \
        } \
        exit(0); \
    } else { \
        int status; \
        waitpid(pid, &status, 0); \
    } \
} while (0)                                

#define assert_eq_custom(left, right, size, pfn) do { \
    int pid = fork(); \
    if (pid == 0) { \
        if (memcmp(left, right, size) == 0) { \
            PRINT_SUCCESS_TEST("OK "); \
        } else { \
            do { \
                if ((pfn) == (NULL)) { \
                    fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
                } else { \
                    DbgFn fn = (DbgFn)pfn; \
                    char *_left = fn(left); \
                    char *_right = fn(right); \
                    fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
                    free(_left); \
                    free(_right); \
                } \
            } while (0); \
        } \
        exit(0); \
    } else { \
        int status; \
        waitpid(pid, &status, 0); \
    } \
} while (0)

#define assert_eq_custom_left_right(left, right, size, pfn_left,pfn_right) do { \
    if (memcmp(left, right, size) == 0) { \
        PRINT_SUCCESS_TEST("OK "); \
    } else { \
        do { \
            if (((pfn_left) != (NULL) && (pfn_right) != (NULL))) { \
                DbgFn fn_left = (DbgFn)pfn_left; \
                DbgFn fn_right = (DbgFn)pfn_right; \
                char *_left = fn_left(left); \
                char *_right = fn_right(right); \
                fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
                free(_left); \
                free(_right); \
            } else if ((pfn_left) != (NULL) && (pfn_right) == NULL) { \
                DbgFn fn_left = (DbgFn)pfn_left; \
                char *_left = fn_left(left); \
                fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, right); \
                free(_left); \
            } else if ((pfn_right) != (NULL) && (pfn_left) == NULL) { \
                DbgFn fn_right = (DbgFn)pfn_right; \
                char *_right = fn_left(right); \
                fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, _right); \
                free(_right); \
            } \
        } while (0); \
    } \
} while (0)


#define assert_ne_custom(left, right, size, pfn) do { \
    int pid = fork(); \
    if (pid == 0) { \
        if (memcmp(left, right, size) != 0) { \
            PRINT_SUCCESS_TEST("OK "); \
        } else { \
            do { \
                if ((pfn) == (NULL)) { \
                    fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, left, right); \
                } else { \
                    DbgFn fn = (DbgFn)pfn; \
                    char *_left = fn(left); \
                    char *_right = fn(right); \
                    fprintf(stderr, RED "\nassertion `left == right` failed\nleft: \"%s\"\nright: \"%s\"\n" RESET, _left, _right); \
                    free(_left); \
                    free(_right); \
                } \
            } while (0); \
        } \
        exit(0); \
    } else { \
        int status; \
        waitpid(pid, &status, 0); \
    } \
} while (0)


#define assert_eq_null_custom(data, pfn) do { \
    int pid = fork(); \
    if (pid == 0) { \
        if (data == NULL) { \
            PRINT_SUCCESS_TEST("OK "); \
        } else { \
            if (pfn != NULL) { \
                DbgFn fn = (DbgFn)pfn; \
                char *_data = fn(data); \
                fprintf(stderr, RED "\nassertion `data == NULL` failed\ndata: \"%s\"\n" RESET , _data); \
                free(_data); \
            } else { \
                fprintf(stderr, RED "\nassertion `data == NULL` failed\ndata: \"%s\"" RESET, data); \
            } \
        } \
        exit(0); \
    } else {\
        int status; \
        waitpid(pid, &status, 0); \
    } \
} while (0)

#define assert_ne_null_custom(data, pfn) do { \
    int pid = fork(); \
    if (pid == 0) { \
        if (data != NULL) { \
            PRINT_SUCCESS_TEST("OK "); \
        } else { \
            if (pfn != NULL) { \
                DbgFn fn = (DbgFn)pfn; \
                char *_data = fn(data); \
                fprintf(stderr, RED "\nassertion `data == NULL` failed\ndata: \"%s\"\n" RESET , _data); \
                free(_data); \
            } else { \
                fprintf(stderr, RED "\nassertion `data == NULL` failed\ndata: \"%s\"" RESET, data); \
            } \
        } \
        exit(0); \
    } else {\
        int status; \
        waitpid(pid, &status, 0); \
    } \
} while (0)

#define d_assert_eq(left,right,size) assert_eq_custom(left,right,size,NULL)
#define assert_eq_null(data) assert_eq_null_custom(data,NULL)
#define d_assert_ne(left,right,size) assert_ne_custom(left,right,size,NULL)
#define assert_ne_null(data) assert_ne_null_custom(data,NULL)
#endif