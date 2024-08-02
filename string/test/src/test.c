#include <dstring.h>
#include <dtest.h>
#include <dutils.h>
#include <string.h>
#include <general_lib.h>
#include <ctype.h>
#include <stdlib.h>

char*   itoa_usize(void* data)
{
    return d_itoa_usize(*((usize*)data));
}

char*   itoa_i32(void* data)
{
    return d_itoa_i32(*((int32*)data));
}

char*   print_d_string(void* _dstring)
{
    DString* dstring = (DString*)_dstring;
    char *str = malloc((sizeof(char) * dstring -> len) + 1);
    memcpy(str, dstring -> string, dstring -> len + 1);
    return str;
}

void    test_d_string_destroy(void)
{
    DString* dstring = d_string_new();
    d_string_destroy(&dstring);
    assert_eq_null_custom(dstring, print_d_string);
}

void    test_d_string_new_from_c_string(void)
{

    char *test_value[] = {"hello", "", "odaidhizahdouazjhduiazduaidiuzahdauzdhuai"};

    for (size_t i = 0; i < 3; i++)
    {
        DString* dstring = d_string_new_from_c_string(test_value[i]);
        usize len = strlen(test_value[i]);
        d_assert_eq(dstring -> string, test_value[i], len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
        d_string_destroy(&dstring);
    }
}

void    test_d_string_new_with_substring(void)
{

    char *str = "Hello world";
    usize pos = 0;
    usize len = 3;
    DString* dstring = d_string_new_with_substring(str, pos, len);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
    
    len = 4;
    pos = 4;
    dstring = d_string_new_with_substring(str, pos, 4);
    str = "o wo";
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);

    pos = 0;
    str = "Hello world";
    len = strlen(str);
    dstring = d_string_new_with_substring(str, pos, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_assert_eq(dstring -> string, str, len);
    d_string_destroy(&dstring);

    pos = 0;
    str = "Hello world";
    len = strlen(str);
    dstring = d_string_new_with_substring(str, pos, MAX_SIZE_T_VALUE);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_assert_eq(dstring -> string, str, len);
    d_string_destroy(&dstring);

    pos = 11;
    str = "Hello world";
    len = 0;
    dstring = d_string_new_with_substring(str, pos, MAX_SIZE_T_VALUE);
    str = "";
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_assert_eq(dstring -> string, str, 1);
    d_string_destroy(&dstring);

    pos = 8445;
    str = "Hello world";
    len = strlen(str);
    dstring = d_string_new_with_substring(str, pos, MAX_SIZE_T_VALUE);
    assert_eq_null_custom(dstring, print_d_string);
    if (dstring)
        d_string_destroy(&dstring);
}

void    test_d_string_resize(void)
{
    char test_value[] = "Hello World!, My name is dieriba.ll";
    usize resize = 5;

    DString* dstring = d_string_new_from_c_string(test_value);
   
    resize = 1000;
    d_string_resize(dstring, resize);
    assert_eq_custom(&dstring -> len, &resize, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_get_capacity(void)
{
    DString* dstring = d_string_new();
    usize capacity = 8;
    usize string_capacity = d_string_get_capacity(dstring);
    assert_eq_custom(&string_capacity, &capacity, sizeof(usize), itoa_usize); //Default capacity is 8
    d_string_destroy(&dstring);
}

void    test_d_string_modify_capacity(void)
{
    DString* dstring = d_string_new();
    usize capacity = 10;
    d_string_modify_capacity(dstring, capacity);
    usize _cap = d_string_get_capacity(dstring);
    assert_eq_custom(&_cap, &capacity, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_push_char(void)
{
    char *test_str_eq = "hello worldhello worldhello worldhello worldhello world";

    DString* dstring = d_string_new();

    for (usize i = 0; i < strlen(test_str_eq); i++)
    {
        usize len = i + 1;
        d_string_push_char(dstring, test_str_eq[i]);
        d_assert_eq(dstring -> string, test_str_eq, len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    }
    d_string_destroy(&dstring);
}

void    test_d_string_push_c_str(void)
{
    char *test_str_arr[] = {"hello", " dieriba", " how", " are you today ", "?"};
    char *test_str_eq = "hello dieriba how are you today ?";
    DString* dstring = d_string_new();
    usize len = 0;
    for (size_t i = 0; i < 5; i++)
    {
        d_string_push_c_str(dstring, test_str_arr[i]);
        len += strlen(test_str_arr[i]);
        d_assert_eq(dstring -> string, test_str_eq, len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    }
    d_string_destroy(&dstring);
}

void    test_d_string_push_str_with_len(void)
{
    char *test_str_arr[] = {"hello", " dieriba", " how", " are you today ", "?"};
    char *test_str_eq = "hello dieriba how are you today ?";
    DString* dstring = d_string_new();
    usize len = 0;
    for (size_t i = 0; i < 5; i++)
    {
        usize str_len = strlen(test_str_arr[i]);
        d_string_push_str_with_len(dstring, test_str_arr[i], str_len);
        len += str_len;
        d_assert_eq(dstring -> string, test_str_eq, len);
        assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    }
    d_string_destroy(&dstring);
}


void    test_d_string_replace_from_str(void)
{
    DString* dstring = d_string_new_from_c_string("Dieriba");
    char *str = "success";   
    usize len = strlen(str);
    d_string_replace_from_str(&dstring, str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_replace_from_dstring(void)
{
    DString* dstring = d_string_new_from_c_string("Dieriba");
    DString* dstring1 = d_string_new_from_c_string("sucess");
    d_string_replace_from_dstring(&dstring, dstring1);
    d_assert_eq(dstring -> string, dstring1 -> string, dstring1 -> len);
    assert_eq_custom(&dstring -> len, &dstring1 -> len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void		test_d_string_compare(void)
{
    DString* dstring1 = d_string_new_from_c_string("hello");
    DString* dstring2 = d_string_new_from_c_string("hello");

    int32 compare = d_string_compare(dstring1, dstring2);
    int32 to_cmp = 0;
    d_assert(compare == 0, &compare, &to_cmp, itoa_i32);

    dstring1 = d_string_replace_from_str(&dstring1, "a");
    dstring2 = d_string_replace_from_str(&dstring2, "b");
    compare = d_string_compare(dstring1, dstring2);
    d_assert(compare < 0, &compare, &to_cmp, itoa_i32);

    dstring1 = d_string_replace_from_str(&dstring1, "b");
    dstring2 = d_string_replace_from_str(&dstring2, "a");
    compare = d_string_compare(dstring1, dstring2);

    d_assert(compare > 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);
}

void    test_d_string_find_char(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
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
    DString* dstring = d_string_new_from_c_string("hello");
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
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

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
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

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

    d_string_destroy(&dstring);
}

usize   is_upper(char c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

void    test_d_string_find_by_predicate(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(&dstring, "bonjour Dieriba");
    haystack = 8;
    pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    dstring = d_string_replace_from_str(&dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_find_by_predicate_from(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_by_predicate_from(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_find_by_predicate_from(dstring, is_upper, 6);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(&dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_by_predicate_from(dstring, is_upper, 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}


void    test_d_string_rfind_by_char(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
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
    DString* dstring = d_string_new_from_c_string("hello");
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
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

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
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

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
    DString* dstring = d_string_new_from_c_string("Bonjour dieriba");

    usize haystack = 0;
    usize pos = d_string_rfind_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(&dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_by_predicate(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_rfind_by_predicate_from(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_rfind_by_predicate_from(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_rfind_by_predicate_from(dstring, is_upper, 13);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(&dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_rfind_by_predicate_from(dstring, is_upper, 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_sub_string_in_place(void)
{
    DString* dstring = d_string_new_from_c_string("Hello World!");
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
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_not_of(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_of(dstring, ' ');
    usize found = 4;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_find_first_not_of(dstring, 'e');
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_not_of_from(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_of_from(dstring, ' ', 5);
    usize found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_find_first_not_of_from(dstring, 'e', 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_find_first_not_of_str(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_of_str(dstring, " bon");
    usize found = 7;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_find_first_not_of_str(dstring, "");
    found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_not_of_str_from(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_of_str_from(dstring, " bonj", 0);
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_find_first_not_of_str_from(dstring, " bonjour", 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_rfind_first_not_of(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_rfind_first_not_of(dstring, ' ');
    usize found = 10;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_rfind_first_not_of(dstring, 'e');
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_first_not_of_from(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour   ");
    usize pos = d_string_rfind_first_not_of_from(dstring, ' ', 5);
    usize found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_rfind_first_not_of_from(dstring, 'e', 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_rfind_first_not_of_str(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_rfind_first_not_of_str(dstring, " bonj");
    usize found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_rfind_first_not_of_str(dstring, " bonjour");
    found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_rfind_first_not_of_str_from(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_rfind_first_not_of_str_from(dstring, " bonj", dstring -> len);
    usize found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(&dstring, "eeeeee");
    pos = d_string_rfind_first_not_of_str_from(dstring, " bonjour", 9);
    found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_trim_left_by_char_in_place(void)
{
    DString* dstring = d_string_new_from_c_string("              bonjour");
    d_string_trim_left_by_char_in_place(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(&dstring, "aaaadieribaaa");
    d_string_trim_left_by_char_in_place(dstring, 'a');
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

usize is_num(char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

void    test_d_string_trim_left_by_predicate_in_place(void)
{
    DString* dstring = d_string_new_from_c_string("DAZDZADDAbonjour");
    d_string_trim_left_by_predicate_in_place(dstring, is_upper);
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(&dstring, "845454554500dieribaaa");
    d_string_trim_left_by_predicate_in_place(dstring, is_num);
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_trim_left_by_char_new(void)
{
    DString* dstring = d_string_new_from_c_string("              bonjour");
    DString* dstring1 = d_string_trim_left_by_char_new(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    
    d_string_destroy(&dstring1);
    d_string_replace_from_str(&dstring, "aaaadieribaaa");
    dstring1 = d_string_trim_left_by_char_new(dstring, 'a');
    
    len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_left_by_predicate_new(void)
{
    DString* dstring = d_string_new_from_c_string("DAZDZADDAbonjour");
    DString* dstring1 = d_string_trim_left_by_predicate_new(dstring, is_upper);
    
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring1);

    str = "dieribaaa";
    d_string_replace_from_str(&dstring, "845454554500dieribaaa");
    dstring1 = d_string_trim_left_by_predicate_new(dstring, is_num);
    
    len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_right_by_char_in_place(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour              ");
    d_string_trim_right_by_char_in_place(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "aaadierib";
    d_string_replace_from_str(&dstring, "aaadieribaaaa");
    d_string_trim_right_by_char_in_place(dstring, 'a');
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_trim_right_by_predicate_in_place(void)
{
    DString* dstring = d_string_new_from_c_string("dabonjourDAZDZADDA");
    d_string_trim_right_by_predicate_in_place(dstring, is_upper);
    char *str = "dabonjour";
    usize len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    str = "dieribaaa";
    d_string_replace_from_str(&dstring, "dieribaaa845454554500");
    d_string_trim_right_by_predicate_in_place(dstring, is_num);
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}


void    test_d_string_trim_right_by_char_new(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour              ");
    DString* dstring1 = d_string_trim_right_by_char_new(dstring, ' ');
    char *str = "bonjour";
    usize len = strlen(str);
    
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    str = "dierib";
    
    d_string_destroy(&dstring1);
    d_string_replace_from_str(&dstring, "dieribaaaa");
    dstring1 = d_string_trim_right_by_char_new(dstring, 'a');
    
    len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_d_string_trim_right_by_predicate_new(void)
{
    DString* dstring = d_string_new_from_c_string("dabonjourDAZDZADDA");
    DString* dstring1 = d_string_trim_right_by_predicate_new(dstring, is_upper);
    
    char *str = "dabonjour";
    usize len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring1);

    str = "dieribaaa";
    d_string_replace_from_str(&dstring, "dieribaaa845454554500");
    dstring1 = d_string_trim_right_by_predicate_new(dstring, is_num);
    
    len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

int main()
{
    TEST("test_string_destroy", test_d_string_destroy(););
    TEST("test_d_string_new_from_c_string", test_d_string_new_from_c_string(););
    TEST("test_d_string_new_with_substring", test_d_string_new_with_substring(););
    TEST("test_d_string_resize", test_d_string_resize(););
    TEST("test_d_string_get_capacity", test_d_string_get_capacity(););
    TEST("test_d_string_modify_capacity", test_d_string_modify_capacity(););
    TEST("test_d_string_push_char", test_d_string_push_char(););
    TEST("test_d_string_push_c_str", test_d_string_push_c_str(););
    TEST("test_d_string_push_str_with_len", test_d_string_push_str_with_len(););
    TEST("test_d_string_replace_from_str", test_d_string_replace_from_str(););
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
    TEST("test_d_string_sub_string_in_place", test_d_string_sub_string_in_place(););
    TEST("test_d_string_find_first_not_of", test_d_string_find_first_not_of(););
    TEST("test_d_string_find_first_not_of_from", test_d_string_find_first_not_of_from(););
    TEST("test_d_string_find_first_not_of_str", test_d_string_find_first_not_of_str(););
    TEST("test_d_string_find_first_not_of_str_from", test_d_string_find_first_not_of_str_from(););
    TEST("test_d_string_rfind_first_not_of", test_d_string_rfind_first_not_of(););
    TEST("test_d_string_rfind_first_not_of_from", test_d_string_rfind_first_not_of_from(););
    TEST("test_d_string_rfind_first_not_of_str", test_d_string_rfind_first_not_of_str(););
    TEST("test_d_string_rfind_first_not_of_str_from", test_d_string_rfind_first_not_of_str_from(););
    TEST("test_d_string_trim_left_by_char_in_place", test_d_string_trim_left_by_char_in_place(););
    TEST("test_d_string_trim_left_by_predicate_in_place", test_d_string_trim_left_by_predicate_in_place(););
    TEST("test_d_string_trim_left_by_char_new", test_d_string_trim_left_by_char_new(););
    TEST("test_d_string_trim_left_by_predicate_new", test_d_string_trim_left_by_predicate_new(););
    TEST("test_d_string_trim_right_by_char_in_place", test_d_string_trim_right_by_char_in_place(););
    TEST("test_d_string_trim_right_by_predicate_in_place", test_d_string_trim_right_by_predicate_in_place(););
    TEST("test_d_string_trim_right_by_char_new", test_d_string_trim_right_by_char_new(););
    TEST("test_d_string_trim_right_by_predicate_new", test_d_string_trim_right_by_predicate_new(););
}