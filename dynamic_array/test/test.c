#include <dutils.h>
#include <stdio.h>
#include <dtest.h>
#include <darray.h>
#include <stdlib.h>
#include <general_lib.h>
#include <string.h>

char*   itoa_usize(void *nb)
{
    return d_itoa_usize((usize)nb);
}

usize get_nb_len(int nb)
{
    bool neg = nb < 0;
    usize i = 0;

    while (nb != 0)
    {
        nb /= 10;
        ++i;
    }

    return i + (i == 0) + (neg == true);
}

usize get_len_arr(int *arr, usize len)
{
    usize len = 0;
    for (size_t i = 0; i < len; i++)
    {
        len += get_nb_len(arr[i]);
    }
    return len;
}

char* print_d_array_int(void* darray)
{
    DArray* array = (DArray*)darray;

    int* arr = array->data;
    usize arr_len = get_len_arr(arr, array->len);
    usize len = arr_len + ((array->len - 1) * 2) + 2 + 1;
    char *str = malloc(sizeof(char) * len);
    str[len - 1] = 0;
    str[0] = '[';
    str[len - 2] = ']';
    usize i = 1;
    for (size_t j = 0; j < arr_len; ++j)
    {
        char *nb = d_itoa_i32_no_alloc(arr[j]);
        usize len = strlen(nb);
        if (j == 0)
            memcpy(str + i, nb, len);
        else
        {
            memcpy(str + i, ", ", 2);
            i += 2;
            memcpy(str + i, nb, len);
        }
        i += len;   
    }
    return str;
}

char*   print_d_pointer_arr_as_string(void* arr)
{
    DPointerArray* array = (DPointerArray*) arr;
    char** tab = (char**)array -> pdata;
    usize size = 0;
    for (size_t i = 0; i < array -> len; i++)
    {
        size += strlen(tab[i]);
    }
    usize len = size + ((array -> len - 1) * 2) + 2 + 1;
    char *str = malloc(sizeof(char) * len);
    str[len - 1] = 0;
    str[len - 2] = ']';
    str[0] = '[';
    usize j = 1;
    for (size_t i = 0; i < array -> len; i++)
    {
        usize len = strlen(tab[i]);
        if (i == 0)
            memcpy(str + j, tab[i], len);
        else if (i != 0 && len > 0)
        {
            memcpy(str + j, ", ", 2);
            j += 2;
            memcpy(str + j, tab[i], len);
        }
        j += len;
    }
    
    return str;
}

void    test_d_array_new(void)
{
    usize len = 4;
    DArray* array = d_array_new(true, sizeof(int), len);
    int arr[] = {0, 0, 0, 0};
    assert_eq_custom(array -> data, arr, sizeof(int) * len, print_d_array_int);
    d_array_destroy(&array);
}

void    test_d_array_destroy(void)
{
    DArray* array = d_array_new(true, sizeof(int), 4);
    d_array_destroy(&array);
    assert_eq_null_custom(array, print_d_array_int);
}

void    test_d_array_push_back(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int32 i = 0; i < 10; i++)
    {
        d_array_push_back(array, i);
    }
    usize len = 10;
    assert_eq_custom(array -> data, arr, sizeof(int) * 10, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);

    int second_arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
                        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
                        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
                        31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
                        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 
                        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
                        61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 
                        71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 
                        81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 
                        91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 
                        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 
                        111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 
                        121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 
                        131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 
                        141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 
                        151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 
                        161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 
                        171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 
                        181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 
                        191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 
                        201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 
                        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 
                        221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 
                        231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 
                        241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 
                        251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 
                        261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 
                        271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 
                        281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 
                        291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 
                        301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 
                        311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 
                        321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 
                        331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 
                        341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 
                        351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 
                        361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 
                        371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 
                        381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 
                        391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 
                        401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 
                        411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 
                        421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 
                        431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 
                        441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 
                        451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 
                        461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 
                        471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 
                        481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 
                        491, 492, 493, 494, 495, 496, 497, 498, 499, 500
 };
    d_array_destroy(&array);
    for (int32 i = 1; i <= 500; i++)
    {
        d_array_push_back(array, i);
    }

    usize len = 500;
    assert_eq_custom(array -> data, second_arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
}

void    test_d_array_append_vals(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    usize len = 10;
    d_array_append_vals(array, arr, sizeof(int) * len);
    assert_eq_custom(array -> data, arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    int second_arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
                        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
                        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
                        31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
                        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 
                        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
                        61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 
                        71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 
                        81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 
                        91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 
                        101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 
                        111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 
                        121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 
                        131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 
                        141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 
                        151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 
                        161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 
                        171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 
                        181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 
                        191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 
                        201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 
                        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 
                        221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 
                        231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 
                        241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 
                        251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 
                        261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 
                        271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 
                        281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 
                        291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 
                        301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 
                        311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 
                        321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 
                        331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 
                        341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 
                        351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 
                        361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 
                        371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 
                        381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 
                        391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 
                        401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 
                        411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 
                        421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 
                        431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 
                        441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 
                        451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 
                        461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 
                        471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 
                        481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 
                        491, 492, 493, 494, 495, 496, 497, 498, 499, 500
 };
    d_array_destroy(&array);
    usize len = 500;
    d_array_append_vals(array, second_arr, len * sizeof(int));
    assert_eq_custom(array -> data, second_arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    d_array_destroy(&array);
}

void    test_d_array_copy(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    usize len = 10;
    d_array_append_vals(array, arr, sizeof(int) * len);
    DArray* copy_array = d_array_copy(array);
    assert_eq_custom(array -> data, copy_array -> data, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &copy_array -> len, sizeof(usize), itoa_usize);
    d_array_destroy(&array);
}

void    test_d_array_get_capacity(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    usize capacity = d_array_get_capacity(array); //default capacity is 4
    usize default_capacity = 4;
    assert_eq_custom(&capacity, &default_capacity, sizeof(usize), itoa_usize);
    d_array_destroy(array);
}

void    test_d_array_increase_capacity(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    usize new_capacity = 500;
    d_array_increase_capacity(array, new_capacity);
    usize capacity = d_array_get_capacity(array);
    assert_eq_custom(&capacity, &new_capacity, sizeof(usize), itoa_usize);
    d_array_destroy(array);
}

void    test_d_array_remove_index_fast(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    int arr[] = {1, 2, 3, 4};
    usize len = 4;
    d_array_append_vals(array, arr, sizeof(int) * len);
    d_array_remove_index_fast(array, 500);
    assert_eq_custom(array -> data, arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    d_array_remove_index_fast(array, 3);
    len = 3;
    int second_arr[] = {1, 2, 3};
    assert_eq_custom(array -> data, second_arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    int third_arr[] = {1, 3};
    len = 2;
    d_array_remove_index_fast(array, 1);
    assert_eq_custom(array -> data, third_arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    int fourth_arr[] = {1};
    len = 1;
    d_array_remove_index_fast(array, 1);
    assert_eq_custom(array -> data, fourth_arr, sizeof(int) * len, print_d_array_int);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    len = 0;
    d_array_remove_index_fast(array, 0);
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    d_array_destroy(&array);
}

void    test_d_array_clear_array(void)
{
    DArray* array = d_array_new(false, sizeof(int), 0);
    int arr[] = {1, 2, 3, 4};
    usize len = 4;
    d_array_append_vals(array, arr, sizeof(int) * len);
    d_array_clear_array(array);
    len = 0;
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    d_array_destroy(&array);
}


void    test_d_pointer_array_new(void)
{
    usize len = 4;
    DPointerArray* array = d_pointer_array_new(len, true);
    char* arr[] = {NULL, NULL, NULL, NULL};
    for (size_t i = 0; i < len; i++)
    {
        char *str = array -> pdata[i];
        d_assert_eq(str, arr, sizeof(void*) * len);
    }
    assert_eq_custom(&array -> len, &len, sizeof(usize), itoa_usize);
    d_pointer_array_destroy(&array);
}

void    test_d_pointer_array_destroy(void)
{
    DPointerArray* array = d_pointer_array_new(0, true);
    d_pointer_array_destroy(&array);
    assert_eq_null(array, print_d_pointer_arr_as_string);
}

void    test_d_pointer_array_push_back(void)
{
    usize len = 3;
    DPointerArray* array = d_pointer_array_new(len, true);
    char* arr[3] = {"bonjour", "dieriba"};
    d_pointer_array_push_back(array, arr[0]);
    d_pointer_array_push_back(array, arr[1]);
    arr[2] = NULL;
}

void    test_d_pointer_array_append_vals(void)
{
    
}

void    test_d_pointer_array_copy(void)
{

}

void    test_d_pointer_array_get_capacity(void)
{

}

void    test_d_pointer_array_increase_capacity(void)
{

}


void    test_d_pointer_array_remove_index_fast(void)
{

}

void    test_d_array_clear_array(void)
{

}

int main(void)
{
    TEST("test_d_array_destroy", test_d_array_destroy(););
    TEST("test_d_array_append_vals", test_d_array_append_vals(););
    TEST("test_d_array_push_back", test_d_array_push_back(););
    TEST("test_d_array_copy", test_d_array_copy(););
    TEST("test_d_array_get_capacity", test_d_array_get_capacity(););
    TEST("test_d_array_increase_capacity", test_d_array_increase_capacity(););
    TEST("test_d_array_remove_index_fast", test_d_array_remove_index_fast(););
    TEST("test_d_array_clear_array", test_d_array_clear_array(););
    TEST("test_d_pointer_array_destroy", test_d_pointer_array_destroy(););
    TEST("test_d_pointer_array_append_vals", test_d_pointer_array_append_vals(););
    TEST("test_d_pointer_array_push_back", test_d_pointer_array_push_back(););
    TEST("test_d_pointer_array_copy", test_d_pointer_array_copy(););
    TEST("test_d_pointer_array_get_capacity", test_d_pointer_array_get_capacity(););
    TEST("test_d_pointer_array_increase_capacity", test_d_pointer_array_increase_capacity(););
    TEST("test_d_pointer_array_remove_index_fast", test_d_pointer_array_remove_index_fast(););
    TEST("test_d_pointer_array_clear_array", test_d_pointer_array_clear_array(););
}