#include "../include/darray.h"
#include <stdio.h>

void    test_d_array_new()
{
    DArray *array = d_array_new(false, sizeof(int), 0);
    
    for (int i = 0; i < 100; i++)
    {
        d_array_push_back(array, &i);
    }


    d_array_remove_index_fast(array, 0);
    int val = d_array_get_val_by_index(array, int, 0);
    for (int i = 0; i < array->len; i++)
    {
        int val = d_array_get_val_by_index(array, int, i);
        printf("[%d] ", val);
    }
}

int main(void)
{
    test_d_array_new();
}