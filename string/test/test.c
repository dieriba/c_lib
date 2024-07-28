#include <dstring.h>
#include <dtest.h>
#include <dutils.h>
#include <string.h>
#include <general_lib.h>
#include <ctype.h>
#include <stdlib.h>

char*   itoa_usize(void* data)
{
    return d_itoa_usize((usize)data);
}

char*   itoa_i32(void* data)
{
    return d_itoa_i32((int32)data);
}

char*   print_struct(void* _dstring)
{
    DString* dstring = (DString*)_dstring;
    char *str = malloc((sizeof(char) * dstring -> len) + 1);
    memcpy(str, dstring -> string, dstring -> len);
    return str;
}

void    test_d_string_destroy(void)
{
    DString* dstring = d_string_new();
    d_string_destroy(&dstring);
    d_assert(dstring == NULL, dstring, NULL, print_struct);
}

void    test_d_string_new_with_string(void)
{

    char *test_value[] = {"hello", "", "odaidhizahdouazjhduiazduaidiuzahdauzdhuai"};

    for (size_t i = 0; i < 3; i++)
    {
        DString* dstring = d_string_new_with_string(test_value[i]);
        usize len = strlen(test_value[i]);
        d_assert_eq(dstring -> string, test_value[i], len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
        d_string_destroy(&dstring);
    }
}

void    test_d_string_new_with_substring(void)
{

    char *str = "Hello world";
    usize from = 0;
    usize len = 3;
    DString* dstring = d_string_new_with_substring(str, from, len);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
    
    from = len + 1;
    dstring = d_string_new_with_substring(str, from, from - 1);
    len = 0;
    str = "";
    d_assert_eq(dstring -> string, str, &len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);

    from = 0;
    str = "Hello world";
    len = strlen(str);
    dstring = d_string_new_with_substring(str, from, len);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);

}

void    test_d_string_resize(void)
{

    char *test_value = "Hello World!, My name is dieriba.ll";
    usize resize = 5;

    DString* dstring = d_string_new_with_string(test_value);
    for (usize i = 0; i < 7; ++i)
    {
        d_string_resize(dstring, resize);
        d_assert_eq(dstring -> string, test_value, resize);
        assert_eq_custom(&dstring -> len, &resize, sizeof(usize), itoa_usize);
        resize += 5;
    }
    resize = 1000;
    d_string_resize(dstring, resize);
    assert_eq_custom(&dstring -> len, &resize, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_get_capacity(void)
{
    DString* dstring = d_string_new();
    usize capacity = 8;
    assert_eq_custom(&dstring -> len, &capacity, sizeof(usize), itoa_usize); //Default capacity is 8
    d_string_destroy(&dstring);
}

void    test_d_string_increase_capacity(void)
{
    DString* dstring = d_string_new();
    usize capacity = 500;
    d_string_increase_capacity(dstring, capacity);
    usize _cap = d_string_get_capacity(dstring);
    assert_eq_custom(&_cap, &capacity, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_push_char(void)
{
    char *test_str_eq = "hello world";

    DString* dstring = d_string_new();

    for (usize i = 0; i < strlen(test_str_eq); i++)
    {
        usize len = i + 1;
        d_string_push_char(dstring, test_str_eq[i]);
        d_assert_eq(dstring -> string, test_str_eq, &len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    }
    d_string_destroy(&dstring);
}

void    test_d_string_push_str(void)
{
    char *test_str_arr[] = {"hello", " dieriba", " how", " are you today ", "?"};
    char *test_str_eq = "hello dieriba how are you today ?";
    DString* dstring = d_string_new();
    usize len = 0;
    for (size_t i = 0; i < 5; i++)
    {
        d_string_push_str(dstring, test_str_arr[i]);
        len += strlen(test_str_arr[i]);
        d_assert_eq(dstring -> string, test_str_eq, len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    }
    d_string_destroy(&dstring);
}

void    test_d_string_copy(void)
{
    char *test_str_arr[] = {"hello", ""};

    for (size_t i = 0; i < 2; i++)
    {
        DString* dstring = d_string_new_with_string(test_str_arr[i]);
        DString* dstring_copy = d_string_copy(dstring);
        usize len = strlen(test_str_arr[i]);
        d_assert_eq(dstring -> string, dstring_copy -> string, len);
        assert_eq_custom(&dstring -> len, &dstring_copy -> len, sizeof(usize), itoa_usize);
        d_string_destroy(&dstring);
        d_string_destroy(&dstring_copy);
    }
}

void    test_d_string_replace_from_str(void)
{
    DString* dstring = d_string_new_with_string("Dieriba");
    char *str = "success";   
    usize len = strlen(str);
    d_string_replace_from_str(dstring, str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_replace_from_dstring(void)
{
    DString* dstring = d_string_new_with_string("Dieriba");
    DString* dstring1 = d_string_new_with_string("success");
    d_string_replace_from_dstring(dstring, dstring1);
    d_assert_eq(dstring -> string, dstring1 -> string, dstring1 -> len);
    assert_eq_custom(&dstring -> len, &dstring1 -> len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void		test_d_string_compare(void)
{
    DString* dstring1 = d_string_new_with_string("hello");
    DString* dstring2 = d_string_new_with_string("hello");

    int32 compare = d_string_compare(dstring1, dstring2);
    int32 to_cmp = 0;
    d_assert(compare == 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);

    dstring1 = d_string_new_with_string("a");
    dstring2 = d_string_new_with_string("b");
    compare = d_string_compare(dstring1, dstring2);
    d_assert(compare < 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);

    dstring1 = d_string_new_with_string("b");
    dstring2 = d_string_new_with_string("a");
    compare = d_string_compare(dstring1, dstring2);

    d_assert(compare > 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);
}

void    test_d_string_find_char(void)
{
    DString* dstring = d_string_new_with_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_char(dstring, 'a');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_find_char(dstring, 'e');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_char_from(void)
{
    DString* dstring = d_string_new_with_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_char_from(dstring, 'a', 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 4;
    pos = d_string_find_char_from(dstring, 'o', 3);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_str(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_find_str(dstring, "riba");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 4;
    pos = d_string_find_str(dstring, "ou");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_str(dstring, "mam");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_str_from(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = MAX_SIZE_T_VALUE;
    usize pos = d_string_find_str_from(dstring, "riba", 14);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 1;
    pos = d_string_find_str_from(dstring, "o", 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    haystack = 4;
    pos = d_string_find_str_from(dstring, "o", 2);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_str_from(dstring, "mam", 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

bool   is_upper(char c)
{
    return (c >= 'A' && c <= 'Z');
}

void    test_d_string_find_by_predicate(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "bonjour Dieriba");
    haystack = 8;
    pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_by_predicate_from(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_by_predicate_from(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_find_by_predicate_from(dstring, is_upper, 6);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_by_predicate_from(dstring, is_upper, 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_rfind_by_char(void)
{
    DString* dstring = d_string_new_with_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_rfind_char(dstring, 'a');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_rfind_char(dstring, 'e');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_by_char_from(void)
{
    DString* dstring = d_string_new_with_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_rfind_char_from(dstring, 'a', 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_rfind_char_from(dstring, 'e', 1);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_str(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_rfind_str(dstring, "riba");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 4;
    pos = d_string_rfind_str(dstring, "ou");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_str(dstring, "mam");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_str_from(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_rfind_str_from(dstring, "riba", 14);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_str_from(dstring, "o", 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    haystack = 1;
    pos = d_string_rfind_str_from(dstring, "o", 2);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_str_from(dstring, "mam", 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_by_predicate(void)
{
    DString* dstring = d_string_new_with_string("Bonjour dieriba");

    usize haystack = 0;
    usize pos = d_string_rfind_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_by_predicate_from(void)
{
    DString* dstring = d_string_new_with_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_rfind_by_predicate_from(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_rfind_by_predicate_from(dstring, is_upper, 13);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_by_predicate_from(dstring, is_upper, 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_sub_string_new(void)
{
    DString* dstring = d_string_new_with_string("Hello World!");
    DString* dstring1 = d_string_sub_string_new(dstring, 0, 5);
    DString* dstring2 = d_string_sub_string_new(dstring, 5, 7);
    char *str = "Hello";
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    d_assert_eq(&dstring1 -> len, &len, sizeof(usize));
    str = " World!";
    len = strlen(str);
    d_assert_eq(dstring2 -> string, str, len);
    d_assert_eq(&dstring2 -> len, &len, sizeof(usize));
    d_string_destroy(dstring);
    d_string_destroy(dstring1);
    d_string_destroy(dstring2);
}

void    test_d_string_sub_string_in_place(void)
{
    DString* dstring = d_string_new_with_string("Hello World!");
    d_string_sub_string_in_place(dstring, 0, 5);
    char *str = "Hello";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    d_assert_eq(&dstring -> len, &len, sizeof(usize));
    d_string_sub_string_in_place(dstring, 2, 3);
    str = "llo";
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    d_assert_eq(&dstring -> len, &len, sizeof(usize));
    d_string_destroy(dstring);
}

void    test_d_string_trim_left_by_char_in_place(void)
{
    DString* dstring = d_string_new_with_string("              bonjour");
    d_string_trim_left_by_char_in_place(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(dstring, "aaaadieribaaa");
    d_string_trim_left_by_char_in_place(dstring, 'a');
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

bool is_num(char c)
{
    return (c >= '0' && c <= '9');
}

void    test_d_string_trim_left_by_predicate_in_place(void)
{
    DString* dstring = d_string_new_with_string("DAZDZADDAbonjour");
    d_string_trim_left_by_predicate_in_place(dstring, is_upper);
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(dstring, "845454554500dieribaaa");
    d_string_trim_left_by_predicate_in_place(dstring, is_num);
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_trim_left_by_char_new(void)
{
    DString* dstring = d_string_new_with_string("              bonjour");
    DString* dstring1 = d_string_trim_left_by_char_in_new(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    
    d_string_destroy(&dstring1);
    d_string_replace_from_str(dstring, "aaaadieribaaa");
    dstring1 = d_string_trim_left_by_char_in_new(dstring, 'a');
    
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_left_by_predicate_new(void)
{
    DString* dstring = d_string_new_with_string("DAZDZADDAbonjour");
    DString* dstring1 = d_string_trim_left_by_predicate_new(dstring, is_upper);
    
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring1);

    str = "dieribaaa";
    d_string_replace_from_str(dstring, "845454554500dieribaaa");
    dstring1 = d_string_trim_left_by_predicate_new(dstring, is_num);
    
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_right_by_char_in_place(void)
{
    DString* dstring = d_string_new_with_string("bonjour              ");
    d_string_trim_right_by_char_in_place(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "aaadierib";
    d_string_replace_from_str(dstring, "aaadieribaaaa");
    d_string_trim_right_by_char_in_place(dstring, 'a');
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

bool is_num(char c)
{
    return (c >= '0' && c <= '9');
}

void    test_d_string_trim_right_by_predicate_in_place(void)
{
    DString* dstring = d_string_new_with_string("dabonjourDAZDZADDA");
    d_string_trim_right_by_predicate_in_place(dstring, is_upper);
    char *str = "dabonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(dstring, "dieribaaa845454554500");
    d_string_trim_right_by_predicate_in_place(dstring, is_num);
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_trim_right_by_char_new(void)
{
    DString* dstring = d_string_new_with_string("bonjour              ");
    DString* dstring1 = d_string_trim_right_by_char_in_new(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    str = "dierib";
    
    d_string_destroy(&dstring1);
    d_string_replace_from_str(dstring, "dieribaaaa");
    dstring1 = d_string_trim_right_by_char_in_new(dstring, 'a');
    
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_right_by_predicate_new(void)
{
    DString* dstring = d_string_new_with_string("dabonjourDAZDZADDA");
    DString* dstring1 = d_string_trim_right_by_predicate_new(dstring, is_upper);
    
    char *str = "dabonjour";
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring1);

    str = "dieribaaa";
    d_string_replace_from_str(dstring, "dieribaaa845454554500");
    dstring1 = d_string_trim_right_by_predicate_new(dstring, is_num);
    
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

int main()
{
    TEST("test_string_destroy", test_d_string_destroy(););
    TEST("test_d_string_new_with_string", test_d_string_new_with_string(););
    TEST("test_d_string_new_with_substring", test_d_string_new_with_substring(););
    TEST("test_d_string_resize", test_d_string_resize(););
    TEST("test_d_string_get_capacity", test_d_string_get_capacity(););
    TEST("test_d_string_increase_capacity", test_d_string_increase_capacity(););
    TEST("test_d_string_push_char", test_d_string_push_char(););
    TEST("test_d_string_push_str", test_d_string_push_str(););
    TEST("test_d_string_copy", test_d_string_copy(););
    TEST("test_d_string_replace", test_d_string_replace(););
    TEST("test_d_string_replace_from_dstring", test_d_string_replace_from_dstring(););
    TEST("test_d_string_compare", test_d_string_compare(););
    TEST("test_d_string_find_char", test_d_string_find_char(););
    TEST("test_d_string_find_char_from", test_d_string_find_char_from(););
    TEST("test_d_string_find_str", test_d_string_find_str(););
    TEST("test_d_string_find_str_from", test_d_string_find_str_from(););
    TEST("test_d_string_find_by_predicate", test_d_string_find_by_predicate(););
    TEST("test_d_string_find_by_predicate_from", test_d_string_find_by_predicate_from(););
    TEST("test_d_string_rfind_by_char", test_d_string_rfind_by_char(););
    TEST("test_d_string_rfind_by_char_from", test_d_string_rfind_by_char_from(););
    TEST("test_d_string_rfind_str", test_d_string_rfind_str(););
    TEST("test_d_string_rfind_str_from", test_d_string_rfind_str_from(););
    TEST("test_d_string_rfind_by_predicate", test_d_string_rfind_by_predicate(););
    TEST("test_d_string_rfind_by_predicate_from", test_d_string_rfind_by_predicate_from(););
    TEST("test_d_string_sub_string_new", test_d_string_sub_string_new(););
    TEST("test_d_string_sub_string_in_place", test_d_string_sub_string_in_place(););
    TEST("test_d_string_trim_left_by_char_in_place", test_d_string_trim_left_by_char_in_place(););
    TEST("test_d_string_trim_left_by_predicate_in_place", test_d_string_trim_left_by_predicate_in_place(););
    TEST("test_d_string_trim_left_by_char_new", test_d_string_trim_left_by_char_new(););
    TEST("test_d_string_trim_left_by_predicate_new", test_d_string_trim_left_by_predicate_new(););
    TEST("test_d_string_trim_right_by_char_in_place", test_d_string_trim_right_by_char_in_place(););
    TEST("test_d_string_trim_right_by_predicate_in_place", test_d_string_trim_right_by_predicate_in_place(););
    TEST("test_d_string_trim_right_by_char_new", test_d_string_trim_right_by_char_new(););
    TEST("test_d_string_trim_right_by_predicate_new", test_d_string_trim_right_by_predicate_new(););
}