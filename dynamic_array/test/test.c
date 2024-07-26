#include <darray.h>
#include <dtypes.h>
#include <dutils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void    print_test_status(char *test_description, char *expected, bool succeded)
{
    if (succeded == true)
    {
        printf(GREEN "Test[%s] succeded\n" RESET, test_description);
        return;
    }
    fprintf(stderr, RED "Test[%s] failed, expected[%s]\n" RESET , test_description, expected);
}

void    test_d_array_appends_val()
{
    u64 arr_length = 4;
    u64 true_arr_length = 100;
    u64 elem_size = sizeof(int);
    DArray *array = d_array_new(false, elem_size, 0);
    int *test_array = malloc(sizeof(int) * true_arr_length);
    for (size_t i = 0; i < arr_length; i++)
    {
        test_array[i] = i;
        d_array_push_back(array, &test_array[i]);
    }
    int passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("push 4 element inside DArray", "[0, 1, 2, 3]", !passed);
    print_test_status("len is array is 4", "array -> len = 4", array -> len == arr_length);
    arr_length = 10;
    for (size_t i = 4; i < arr_length; i++)
    {
        test_array[i] = i;
    }
    array =  d_array_append_vals(array, &test_array[4], 6);
    passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("append 6 more element to the array", "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]", !passed);
    print_test_status("len is array is 10", "array -> len = 10", array -> len == arr_length);
    for (size_t i = arr_length; i < true_arr_length; i++)
    {
        test_array[i] = i;
    }
    array =  d_array_append_vals(array, &test_array[10], true_arr_length - arr_length);
    passed = memcmp(array -> data, test_array, true_arr_length);
    print_test_status("append 89 more element to the array", "[0 to 99]", !passed);
    print_test_status("len is array is 100", "array -> len = 100", array -> len == true_arr_length);
    free(test_array);
    test_array = NULL;
    d_array_destroy_d_array(&array);
}

void    test_d_array_remove_index_fast()
{
    u64 arr_length = 4;
    u64 elem_size = sizeof(int);
    DArray *array = d_array_new(false, elem_size, 0);
    int *test_array = malloc(sizeof(int) * arr_length);
    for (size_t i = 0; i < arr_length; i++)
    {
        test_array[i] = i;
    }
    d_array_append_vals(array, test_array, arr_length);
    d_array_remove_index_fast(array, 4);
    int passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("remove element that does not exist", "[0, 1, 2, 3]", !passed);
    print_test_status("len is array is 4", "array -> len = 4", array -> len == arr_length);

    d_array_remove_index_fast(array, 2);
    arr_length = 3;
    test_array[2] = 3;
    passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("remove element at index 2", "[0, 1, 3]", !passed);
    print_test_status("len is array is 3", "array -> len = 3", array -> len == arr_length);

    d_array_remove_index_fast(array, 2);
    arr_length = 2;
    passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("remove element at index 2", "[0, 1]", !passed);
    print_test_status("len is array is 2", "array -> len = 2", array -> len == arr_length);

    d_array_remove_index_fast(array, 2);
    print_test_status("len is array is 2", "array -> len = 2", array -> len == arr_length);

    d_array_remove_index_fast(array, 0);
    arr_length = 1;
    test_array[0] = 1;
    passed = memcmp(array -> data, test_array, arr_length);
    print_test_status("remove element at index 0", "[1]", !passed);
    print_test_status("len is array is 1", "array -> len = 1", array -> len == arr_length);

    free(test_array);
    d_array_destroy_d_array(&array);
}

void    test_d_array_destroy_d_array()
{
    DArray *array = d_array_new(false, sizeof(int), 0);
    d_array_destroy_d_array(&array);
    print_test_status("array should be null", "NULL",array == NULL);
}

int main(void)
{
    test_d_array_appends_val();
    test_d_array_remove_index_fast();
    test_d_array_destroy_d_array();
}