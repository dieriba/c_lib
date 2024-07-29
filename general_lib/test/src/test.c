#include <dtest.h>
#include <dtypes.h>
#include <general_lib.h>
#include <string.h>

void test_assert_i32(char *nbr, int32 nb)
{
    char *res = d_itoa_i32(nb);
    d_assert_eq(res, nbr, strlen(nbr));
    free(res);
}

void test_assert_i32_no_alloc(char *nbr, int32 nb)
{
    char *res = d_itoa_i32_no_alloc(nb);
    d_assert_eq(res, nbr, strlen(nbr));
}

void test_assert_usize(char *nbr, usize nb)
{
    char *res = d_itoa_usize(nb);
    d_assert_eq(res, nbr, strlen(nbr));
    free(res);
}

void test_assert_usize_no_alloc(char *nbr, usize nb)
{
    char *res = d_itoa_usize_no_alloc(nb);
    d_assert_eq(res, nbr, strlen(nbr));
}

void test_d_itoa_i32(void)
{
    test_assert_i32("0", 0);
    test_assert_i32("-1", -1);
    test_assert_i32("1", 1);
    test_assert_i32("-2147483648", -2147483648);
    test_assert_i32("2147483647", 2147483647);
    test_assert_i32("10", 10);
    test_assert_i32("100", 100);
}

void test_d_itoa_usize(void)
{
    test_assert_usize("0", 0);
    test_assert_usize("1", 1);
    test_assert_usize("10", 10);
    test_assert_usize("100", 100);
    test_assert_usize("10000", 10000);
    test_assert_usize("10400", 10400);
    test_assert_usize("18446744073709551615", MAX_SIZE_T_VALUE);
}

void test_d_itoa_i32_no_alloc(void)
{
    test_assert_i32_no_alloc("0", 0);
    test_assert_i32_no_alloc("-1", -1);
    test_assert_i32_no_alloc("1", 1);
    test_assert_i32_no_alloc("-2147483648", -2147483648);
    test_assert_i32_no_alloc("2147483647", 2147483647);
    test_assert_i32_no_alloc("10", 10);
    test_assert_i32_no_alloc("100", 100);
}

void test_d_itoa_usize_no_alloc(void)
{
    test_assert_usize_no_alloc("0", 0);
    test_assert_usize_no_alloc("1", 1);
    test_assert_usize_no_alloc("10", 10);
    test_assert_usize_no_alloc("100", 100);
    test_assert_usize_no_alloc("10000", 10000);
    test_assert_usize_no_alloc("10400", 10400);
    test_assert_usize_no_alloc("18446744073709551615", MAX_SIZE_T_VALUE);
}

int main()
{
    TEST("test_d_itoa_i32", test_d_itoa_i32(););
    TEST("test_d_itoa_usize", test_d_itoa_usize(););
    TEST("test_d_itoa_i32_no_alloc", test_d_itoa_i32_no_alloc(););
    TEST("test_d_itoa_usize_no_alloc", test_d_itoa_usize_no_alloc(););
}