#include <dtest.h>
#include <dtypes.h>
#include <general_lib.h>
#include <string.h>

char*   itoa_usize(void* data)
{
    return d_itoa_usize(*((usize*)data));
}

char*   itoa_i32(void* data)
{
    return d_itoa_i32(*((int32*)data));
}

void test_assert_i32(char *nbr, int32 nb)
{
    char *res = d_itoa_i32(nb);
    d_assert_eq(res, nbr, strlen(nbr));
    free(res);
}

void test_assert_i32_no_alloc(char *nbr, int32 nb)
{
    char buffer[12];
    d_itoa_i32_no_alloc(nb, buffer);
    d_assert_eq(buffer, nbr, strlen(nbr));
}

void test_assert_usize(char *nbr, usize nb)
{
    char *res = d_itoa_usize(nb);
    d_assert_eq(res, nbr, strlen(nbr));
    free(res);
}

void test_assert_usize_no_alloc(char *nbr, usize nb)
{
    char buffer[22];
    d_itoa_usize_no_alloc(nb, buffer);
    d_assert_eq(buffer, nbr, strlen(nbr));
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

void test_d_substr(void)
{
    char* str = "bonjour c'est dieriba";
    char* sub_str = d_substr(str, 25, 0);
    assert_eq_null(sub_str);
    
    sub_str = d_substr(NULL, 2, 0);
    assert_eq_null(sub_str);
    
    usize sub_str_len;
    usize str_len;

   

    sub_str = d_substr(str, 8, 5);
    sub_str_len = strlen(sub_str);
    char* res = "c'est";
    str_len = strlen(res);
    d_assert_eq(sub_str, res, strlen(res));
    assert_eq_custom(&sub_str_len, &str_len, sizeof(usize), itoa_usize);
    free(sub_str);

    sub_str = d_substr(str, 0, 0);
    res = "";
    sub_str_len = strlen(sub_str);
    str_len = strlen(res);
    d_assert_eq(sub_str, res, strlen(res));
    assert_eq_custom(&sub_str_len, &str_len, sizeof(usize), itoa_usize);
    free(sub_str);
}

int main()
{
    TEST("test_d_itoa_i32", test_d_itoa_i32(););
    TEST("test_d_itoa_usize", test_d_itoa_usize(););
    TEST("test_d_itoa_i32_no_alloc", test_d_itoa_i32_no_alloc(););
    TEST("test_d_itoa_usize_no_alloc", test_d_itoa_usize_no_alloc(););
    TEST("test_d_substr", test_d_substr(););
}