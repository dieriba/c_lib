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

usize is_num(char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

usize   is_upper(char c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
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

void    test_d_string_new_from_dstring(void)
{

    char *test_value[] = {"hello", "", "odaidhizahdouazjhduiazduaidiuzahdauzdhuai"};

    for (size_t i = 0; i < 3; i++)
    {
        DString* dstring1 = d_string_new_from_c_string(test_value[i]);
        DString* dstring2 = d_string_new_from_dstring(dstring1);
        usize len = strlen(test_value[i]);
        d_assert_eq(dstring1 -> string, dstring2 -> string, len);
        assert_eq_custom(&dstring1 -> len, &dstring2 -> len, sizeof(usize), itoa_usize);
        d_string_destroy(&dstring1);
        d_string_destroy(&dstring2);
    }
}


void test_d_string_substr(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour c'est dieriba");
    char* sub_str = d_string_substr(dstring, 25, 0);
    assert_eq_null(sub_str);
    
    usize sub_str_len;
    usize str_len;

   

    sub_str = d_string_substr(dstring, 8, 5);
    sub_str_len = strlen(sub_str);
    char* res = "c'est";
    str_len = strlen(res);
    d_assert_eq(sub_str, res, strlen(res));
    assert_eq_custom(&sub_str_len, &str_len, sizeof(usize), itoa_usize);
    free(sub_str);

    sub_str = d_string_substr(dstring, 0, 0);
    res = "";
    sub_str_len = strlen(sub_str);
    str_len = strlen(res);
    d_assert_eq(sub_str, res, strlen(res));
    assert_eq_custom(&sub_str_len, &str_len, sizeof(usize), itoa_usize);
    free(sub_str);
}

void test_d_string_strdup(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour c'est dieriba");
    char* dup = d_string_strdup(dstring);
    usize len = strlen(dup);
    d_assert_eq(dup, dstring -> string, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    free(dup);
    d_string_replace_from_str(dstring, "");
    dup = d_string_strdup(dstring);
    len = strlen(dup);
    d_assert_eq(dup, dstring -> string, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
    free(dup);
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

void    test_d_string_push_str_of_dstring(void)
{
    char *test_str_arr[] = {"hello", " dieriba", " how", " are you today ", "?"};
    char *test_str_eq = "hello dieriba how are you today ?";
    DString* dstring1 = d_string_new_from_c_string(test_str_eq);
    DString* dstring2 = d_string_new_from_c_string("wsh ");
    test_str_eq = "wsh hello dieriba how are you today ?";
    d_string_push_str_of_dstring(dstring2, dstring1);
    d_assert_eq(dstring2 -> string, test_str_eq, strlen(test_str_eq));
    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);
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
    d_string_replace_from_str(dstring, str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_replace_from_dstring(void)
{
    DString* dstring = d_string_new_from_c_string("Dieriba");
    DString* dstring1 = d_string_new_from_c_string("sucess");
    d_string_replace_from_dstring(dstring, dstring1);
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

    dstring1 = d_string_replace_from_str(dstring1, "a");
    dstring2 = d_string_replace_from_str(dstring2, "b");
    compare = d_string_compare(dstring1, dstring2);
    d_assert(compare < 0, &compare, &to_cmp, itoa_i32);

    dstring1 = d_string_replace_from_str(dstring1, "b");
    dstring2 = d_string_replace_from_str(dstring2, "a");
    compare = d_string_compare(dstring1, dstring2);

    d_assert(compare > 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
    d_string_destroy(&dstring2);
}

void		test_d_string_compare_against_c_str(void)
{
    DString* dstring1 = d_string_new_from_c_string("hello");

    int32 compare = d_string_compare_againg_c_string(dstring1, "hello");
    int32 to_cmp = 0;
    d_assert(compare == 0, &compare, &to_cmp, itoa_i32);

    dstring1 = d_string_replace_from_str(dstring1, "a");
    compare = d_string_compare_againg_c_string(dstring1, "b");
    d_assert(compare < 0, &compare, &to_cmp, itoa_i32);

    dstring1 = d_string_replace_from_str(dstring1, "b");
    compare = d_string_compare_againg_c_string(dstring1, "a");

    d_assert(compare > 0, &compare, &to_cmp, itoa_i32);

    d_string_destroy(&dstring1);
}

void    test_d_string_starts_with_char(void)
{
    DString* dstring = d_string_new_from_c_string("   \t\n\r   bonjour    ");
    usize pos = d_string_starts_with_char(dstring, 'b', " \n\r\t");
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    pos = d_string_starts_with_char(dstring, 'b', NULL);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "bonjour");
    pos = d_string_starts_with_char(dstring, 'b', NULL);
    found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_ends_with_char(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour     \t\n\r   ");
    usize pos = d_string_ends_with_char(dstring, 'r', " \n\r\t");
    usize found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    pos = d_string_ends_with_char(dstring, 'r', NULL);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "bonjour");
    pos = d_string_ends_with_char(dstring, 'r', NULL);
    found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_starts_with_str(void)
{
    DString* dstring = d_string_new_from_c_string("   \t\n\r   // bonjour    ");
    usize pos = d_string_starts_with_str(dstring, "//", " \n\r\t");
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    pos = d_string_starts_with_str(dstring, "//", NULL);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "hello bonjour");
    pos = d_string_starts_with_str(dstring, "hello", NULL);
    found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "      bo");
    pos = d_string_starts_with_str(dstring, "bon", NULL);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_ends_with_str(void)
{
    DString* dstring = d_string_new_from_c_string("   \t\n\r    bonjour  .  ");
    usize pos = d_string_ends_with_str(dstring, ".", " \n\r\t");
    usize found = 19;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    pos = d_string_ends_with_str(dstring, ".", NULL);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "hello bonjour");
    pos = d_string_ends_with_str(dstring, "bonjour", NULL);
    found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_char_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_first_matching_char_from_start(dstring, 'a');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_find_first_matching_char_from_start(dstring, 'e');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_char_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_first_matching_char_from_index(dstring, 'a', 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 4;
    pos = d_string_find_first_matching_char_from_index(dstring, 'o', 3);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_str_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_find_first_matching_str_from_start(dstring, "riba");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 4;
    pos = d_string_find_first_matching_str_from_start(dstring, "ou");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_first_matching_str_from_start(dstring, "mam");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = MAX_SIZE_T_VALUE;
    usize pos = d_string_find_first_matching_str_from_index(dstring, "riba", 14);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 1;
    pos = d_string_find_first_matching_str_from_index(dstring, "o", 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    haystack = 4;
    pos = d_string_find_first_matching_str_from_index(dstring, "o", 2);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_first_matching_str_from_index(dstring, "mam", 5);

    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_predicate_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_first_matching_predicate_from_start(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "bonjour Dieriba");
    haystack = 8;
    pos = d_string_find_first_matching_predicate_from_start(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    dstring = d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_first_matching_predicate_from_start(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_find_first_matching_predicate_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_first_matching_predicate_from_index(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_find_first_matching_predicate_from_index(dstring, is_upper, 6);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_first_matching_predicate_from_index(dstring, is_upper, 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}


void    test_d_string_find_last_matching_char_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_last_matching_char_from_end(dstring, 'a');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_find_last_matching_char_from_end(dstring, 'e');
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_matching_char_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("hello");
    usize haystack = MAX_VALUE_SIZE_T;
    usize pos = d_string_find_last_matching_char_from_index(dstring, 'a', 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    haystack = 1;
    pos = d_string_find_last_matching_char_from_index(dstring, 'e', 1);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_matching_str_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_find_last_matching_str_from_end(dstring, "riba");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 4;
    pos = d_string_find_last_matching_str_from_end(dstring, "ou");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_last_matching_str_from_end(dstring, "mam");
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_matching_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 11;
    usize pos = d_string_find_last_matching_str_from_index(dstring, "riba", 14);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_last_matching_str_from_index(dstring, "o", 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    haystack = 1;
    pos = d_string_find_last_matching_str_from_index(dstring, "o", 2);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_last_matching_str_from_index(dstring, "mam", 5);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_matching_predicate_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour dieriba");

    usize haystack = 0;
    usize pos = d_string_find_last_matching_predicate_from_end(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_last_matching_predicate_from_end(dstring, is_upper);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_find_last_matching_predicate_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("Bonjour Dieriba");

    usize haystack = 0;
    usize pos = d_string_find_last_matching_predicate_from_index(dstring, is_upper, 0);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    haystack = 8;
    pos = d_string_find_last_matching_predicate_from_index(dstring, is_upper, 13);
    d_assert(pos == haystack, &pos, &haystack, itoa_usize);
    
    d_string_replace_from_str(dstring, "dieriba");
    haystack = MAX_SIZE_T_VALUE;
    pos = d_string_find_last_matching_predicate_from_index(dstring, is_upper, 5);
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

void    test_d_string_find_first_not_matching_char_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_matching_char_from_index(dstring, ' ', 5);
    usize found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_not_matching_char_from_index(dstring, 'e', 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_not_matching_char_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_not_matching_char_from_start(dstring, ' ');
    usize found = 4;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_not_matching_char_from_start(dstring, 'e');
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_char_not_in_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_char_not_in_str_from_index(dstring, " bonj", 0);
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_char_not_in_str_from_index(dstring, " bonjour", 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_char_not_in_str_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_char_not_in_str_from_start(dstring, " bon");
    usize found = 7;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_char_not_in_str_from_start(dstring, "");
    found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_char_in_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_char_in_str_from_index(dstring, " bonj", 0);
    usize found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_char_in_str_from_index(dstring, " bonjour", 0);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    pos = d_string_find_first_char_in_str_from_index(dstring, " bonjour", MAX_SIZE_T_VALUE);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_first_char_in_str_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_first_char_in_str_from_start(dstring, "bon");
    usize found = 4;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_first_char_in_str_from_start(dstring, "");
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_not_matching_char_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour   ");
    usize pos = d_string_find_last_not_matching_char_from_index(dstring, ' ', 5);
    usize found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_not_matching_char_from_index(dstring, 'e', 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_not_matching_char_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("    bonjour");
    usize pos = d_string_find_last_not_matching_char_from_end(dstring, ' ');
    usize found = 10;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_not_matching_char_from_end(dstring, 'e');
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_char_not_in_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_find_last_char_not_in_str_from_index(dstring, " bonj", dstring -> len);
    usize found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_char_not_in_str_from_index(dstring, " bonjour", 9);
    found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_char_not_in_str_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_find_last_char_not_in_str_from_end(dstring, " bonj");
    usize found = 6;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_char_not_in_str_from_end(dstring, " bonjour");
    found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_char_in_str_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_find_last_char_in_str_from_index(dstring, " buonj", 5);
    usize found = 5;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_char_in_str_from_index(dstring, " bonjour", 9);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}

void    test_d_string_find_last_char_in_str_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour jn");
    usize pos = d_string_find_last_char_in_str_from_end(dstring, " bonj");
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "eeeeee");
    pos = d_string_find_last_char_in_str_from_end(dstring, " bonjour");
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
}



void    test_d_string_find_first_not_matching_predicate_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("1234d56789dieriba");
    usize pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_num, 9);
    usize found = 10;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_num, 2);
    found = 4;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "");
    pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_upper, 0);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_upper, 70);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "AB56789");

    pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_num, 0);
    found = 0;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_first_not_matching_predicate_from_index(dstring, is_num, 2);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_find_first_not_matching_predicate_from_start(void)
{
    DString* dstring = d_string_new_from_c_string("123456789dieriba");
    usize pos = d_string_find_first_not_matching_predicate_from_start(dstring, is_num);
    usize found = 9;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);
    d_string_replace_from_str(dstring, "");
    pos = d_string_find_first_not_matching_predicate_from_start(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_first_not_matching_predicate_from_start(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "ABDEFFDAD");

    pos = d_string_find_first_not_matching_predicate_from_start(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);


    d_string_destroy(&dstring);
}

void    test_d_string_find_last_not_matching_predicate_from_index(void)
{
    DString* dstring = d_string_new_from_c_string("die5riba568");
    usize pos = d_string_find_last_not_matching_predicate_from_index(dstring, is_num, 9);
    usize found = 7;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_last_not_matching_predicate_from_index(dstring, is_num, 3);
    found = 2;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "ABD");
    pos = d_string_find_last_not_matching_predicate_from_index(dstring, is_upper, 0);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_last_not_matching_predicate_from_index(dstring, is_upper, 70);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "ABDDEFDE65");
    pos = d_string_find_last_not_matching_predicate_from_index(dstring, is_upper, 7);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_destroy(&dstring);
}

void    test_d_string_find_last_not_matching_predicate_from_end(void)
{
    DString* dstring = d_string_new_from_c_string("die5riba568");
    usize pos = d_string_find_last_not_matching_predicate_from_end(dstring, is_num);
    usize found = 7;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_last_not_matching_predicate_from_end(dstring, is_num);
    found = 7;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "");
    pos = d_string_find_last_not_matching_predicate_from_end(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    pos = d_string_find_last_not_matching_predicate_from_end(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
    assert_eq_custom(&pos, &found, sizeof(usize), itoa_usize);

    d_string_replace_from_str(dstring, "ABDDEFDE");
    pos = d_string_find_last_not_matching_predicate_from_end(dstring, is_upper);
    found = MAX_SIZE_T_VALUE;
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
    d_string_replace_from_str(dstring, "aaaadieribaaa");
    d_string_trim_left_by_char_in_place(dstring, 'a');
    len = strlen(str);
    d_assert_eq(dstring -> string, str, len);
    assert_eq_custom(&dstring -> len, &len, sizeof(usize), itoa_usize);
    d_string_destroy(&dstring);
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
    d_string_replace_from_str(dstring, "845454554500dieribaaa");
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
    d_string_replace_from_str(dstring, "aaaadieribaaa");
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
    d_string_replace_from_str(dstring, "845454554500dieribaaa");
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
    d_string_replace_from_str(dstring, "aaadieribaaaa");
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
    d_string_replace_from_str(dstring, "dieribaaa845454554500");
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
    d_string_replace_from_str(dstring, "dieribaaaa");
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
    d_string_replace_from_str(dstring, "dieribaaa845454554500");
    dstring1 = d_string_trim_right_by_predicate_new(dstring, is_num);
    
    len = strlen(str);
    d_assert_eq(dstring1 -> string, str, len);
    assert_eq_custom(&dstring1 -> len, &len, sizeof(usize), itoa_usize);
    
    d_string_destroy(&dstring);
    d_string_destroy(&dstring1);
}

void    test_splitting_by_char(DString* dstring, char **tab, usize nb_test, char c)
{
    printf("Start Test [%lu]\n", nb_test);
    DPointerArray* arr = d_string_split_by_char(dstring, c);
    if (tab[0] == NULL)
    {
        usize len = 0;
        assert_eq_null(arr -> pdata[0]);
        assert_eq_custom(&arr -> len, &len, sizeof(usize), itoa_usize);
    }
    else
    {
        for (usize i = 0; tab[i] != NULL; ++i)
        {
            char* str = (char*)arr -> pdata[i];
            usize tab_len = strlen(tab[i]);
            usize str_len = strlen(str);
            d_assert_eq(str, tab[i], tab_len);
            assert_eq_custom(&str_len, &tab_len, sizeof(usize), itoa_usize);
        }
    }
    printf("\nEnd Test [%lu]\n", nb_test);
    d_pointer_array_destroy(&arr);
}

void    test_splitting_by_char_of_str(DString* dstring, char **tab, usize nb_test, const char* delims)
{
    printf("Start Test [%lu]\n", nb_test);
    DPointerArray* arr = d_string_split_by_char_of_str(dstring, delims);
    if (tab[0] == NULL)
    {
        usize len = 0;
        assert_eq_null(arr -> pdata[0]);
        assert_eq_custom(&arr -> len, &len, sizeof(usize), itoa_usize);
    }
    else
    {
        for (usize i = 0; tab[i] != NULL; ++i)
        {
            char* str = arr -> pdata[i];
            usize tab_len = strlen(tab[i]);
            usize str_len = strlen(str);
            d_assert_eq(str, tab[i], tab_len);
            assert_eq_custom(&str_len, &tab_len, sizeof(usize), itoa_usize);
        }
    }
    printf("End Test [%lu]\n", nb_test);
    d_pointer_array_destroy(&arr);
}

void    test_d_string_split_by_char(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour");
    usize nb_test = 0;

    d_string_replace_from_str(dstring, "ccc");
    char *tab0[] = {NULL};
    test_splitting_by_char(dstring, tab0, nb_test++, 'c');
    d_string_replace_from_str(dstring, "bonjour");

    char *tab1[] = {"bonjour", NULL};
    test_splitting_by_char(dstring, tab1, nb_test++, 'c');
    
    char *tab2[] = {"b", "nj", "ur", NULL};
    test_splitting_by_char(dstring, tab2, nb_test++, 'o');
    
    char *tab3[] = {"onjour", NULL};
    test_splitting_by_char(dstring, tab3, nb_test++, 'b');
    
    char *tab4[] = {"bonjou", NULL};
    test_splitting_by_char(dstring, tab4, nb_test++, 'r');
    
    d_string_replace_from_str(dstring, "");
    char *tab5[] = {NULL};
    test_splitting_by_char(dstring, tab5, nb_test++, 'r');

    char *tab6[] = {NULL};
    test_splitting_by_char(dstring, tab6, nb_test++, '\0');

    d_string_replace_from_str(dstring, "ab");

    char *tab7[] = {"a", NULL};
    test_splitting_by_char(dstring, tab7, nb_test++, 'b');

    char *tab8[] = {"b", NULL};
    test_splitting_by_char(dstring, tab8, nb_test++, 'a');

    d_string_replace_from_str(dstring, "hello,world;this is:a,test");

    char *tab9[] = {"hello", "world;this is:a", "test", NULL};
    test_splitting_by_char(dstring, tab9, nb_test++, ',');

    d_string_replace_from_str(dstring, ".......,,    ad ...,,dad da dadoidz .");
    char *tab10[] = {".......,,", "ad", "...,,dad", "da", "dadoidz", ".", NULL};
    test_splitting_by_char(dstring, tab10, nb_test++, ' ');

    d_string_destroy(&dstring);
}

void    test_d_string_split_by_char_of_str(void)
{
    DString* dstring = d_string_new_from_c_string("bonjour");
    usize nb_test = 1;

    char *tab1[] = {"bo","our", NULL};
    test_splitting_by_char_of_str(dstring, tab1, nb_test++, "nj");
    
    char *tab2[] = {"b", "nj", "r", NULL};
    test_splitting_by_char_of_str(dstring, tab2, nb_test++, "ou");
    
    char *tab3[] = {"onjou", NULL};
    test_splitting_by_char_of_str(dstring, tab3, nb_test++, "rb");
    
    char *tab4[] = {"bonjou", NULL};
    test_splitting_by_char_of_str(dstring, tab4, nb_test++, "r");
    
    d_string_replace_from_str(dstring, "");
    char *tab5[] = {NULL};
    test_splitting_by_char_of_str(dstring, tab5, nb_test++, "e");

    char *tab6[] = {NULL};
    test_splitting_by_char_of_str(dstring, tab6, nb_test++, "\0");

    d_string_replace_from_str(dstring, "ab");

    char *tab7[] = {NULL};
    test_splitting_by_char_of_str(dstring, tab7, nb_test++, "ab");

    char *tab8[] = {"b", NULL};
    test_splitting_by_char_of_str(dstring, tab8, nb_test++, "a");

    d_string_replace_from_str(dstring, "hello,world;this is:a,test");

    char *tab9[] = {"hello", "world", "this", "is", "a", "test", NULL};
    test_splitting_by_char_of_str(dstring, tab9, nb_test++, ",;: ");

    d_string_replace_from_str(dstring, ".......,,    ad ...,,dad da dadoidz .");
    char *tab10[] = {"ad", "dad", "da", "dadoidz", NULL};
    test_splitting_by_char_of_str(dstring, tab10, nb_test++, " .,");

    d_string_destroy(&dstring);
}


void    test_d_string_convert_as_d_array(void)
{

}

int main()
{
    TEST("test_string_destroy", test_d_string_destroy(););
    TEST("test_d_string_new_from_c_string", test_d_string_new_from_c_string(););
    TEST("test_d_string_new_from_dstring", test_d_string_new_from_dstring(););
    TEST("test_d_string_strdup", test_d_string_strdup(););
    TEST("test_d_string_substr", test_d_string_substr(););
    TEST("test_d_string_new_with_substring", test_d_string_new_with_substring(););
    TEST("test_d_string_sub_string_in_place", test_d_string_sub_string_in_place(););
    TEST("test_d_string_get_capacity", test_d_string_get_capacity(););
    TEST("test_d_string_resize", test_d_string_resize(););
    TEST("test_d_string_modify_capacity", test_d_string_modify_capacity(););
    TEST("test_d_string_push_char", test_d_string_push_char(););
    TEST("test_d_string_push_str_with_len", test_d_string_push_str_with_len(););
    TEST("test_d_string_push_c_str", test_d_string_push_c_str(););
    TEST("test_d_string_push_str_of_dstring", test_d_string_push_str_of_dstring(););
    TEST("test_d_string_replace_from_str", test_d_string_replace_from_str(););
    TEST("test_d_string_replace_from_dstring", test_d_string_replace_from_dstring(););
    TEST("test_d_string_compare", test_d_string_compare(););
    TEST("test_d_string_compare_againt_c_str", test_d_string_compare_against_c_str(););
    
    TEST("test_d_string_starts_with_char", test_d_string_starts_with_char(););
    TEST("test_d_string_ends_with_char", test_d_string_ends_with_char(););
    TEST("test_d_string_starts_with_str", test_d_string_starts_with_str(););
    TEST("test_d_string_ends_with_str", test_d_string_ends_with_str(););

    TEST("test_d_string_find_first_matching_char_from_index", test_d_string_find_first_matching_char_from_index(););
    TEST("test_d_string_find_first_matching_char_from_start", test_d_string_find_first_matching_char_from_start(););
    
    TEST("test_d_string_find_first_not_matching_char_from_index", test_d_string_find_first_not_matching_char_from_index(););
    TEST("test_d_string_find_first_not_matching_char_from_start", test_d_string_find_first_not_matching_char_from_start(););
    
    TEST("test_d_string_find_last_matching_char_from_end", test_d_string_find_last_matching_char_from_end(););
    TEST("test_d_string_find_last_matching_char_from_index", test_d_string_find_last_matching_char_from_index(););

    TEST("test_d_string_find_last_not_matching_char_from_end", test_d_string_find_last_not_matching_char_from_end(););
    TEST("test_d_string_find_last_not_matching_char_from_index", test_d_string_find_last_not_matching_char_from_index(););

    
    TEST("test_d_string_find_first_matching_str_from_start", test_d_string_find_first_matching_str_from_start(););
    TEST("test_d_string_find_first_matching_str_from_index", test_d_string_find_first_matching_str_from_index(););
    
    TEST("test_d_string_find_last_matching_str_from_end", test_d_string_find_last_matching_str_from_end(););
    TEST("test_d_string_find_last_matching_str_from_index", test_d_string_find_last_matching_str_from_index(););
    
    TEST("test_d_string_find_first_char_in_str_from_index", test_d_string_find_first_char_in_str_from_index(););
    TEST("test_d_string_find_first_char_in_str_from_start", test_d_string_find_first_char_in_str_from_start(););
    
    TEST("test_d_string_find_first_char_not_in_str_from_index", test_d_string_find_first_char_not_in_str_from_index(););
    TEST("test_d_string_find_first_char_not_in_str_from_start", test_d_string_find_first_char_not_in_str_from_start(););
    
    TEST("test_d_string_find_last_char_in_str_from_index", test_d_string_find_last_char_in_str_from_index(););
    TEST("test_d_string_find_last_char_in_str_from_end", test_d_string_find_last_char_in_str_from_end(););

    TEST("test_d_string_find_last_char_not_in_str_from_index", test_d_string_find_last_char_not_in_str_from_index(););
    TEST("test_d_string_find_last_char_not_in_str_from_end", test_d_string_find_last_char_not_in_str_from_end(););

    TEST("test_d_string_find_first_matching_predicate_from_index", test_d_string_find_first_matching_predicate_from_index(););
    TEST("test_d_string_find_first_matching_predicate_from_start", test_d_string_find_first_matching_predicate_from_start(););
    
    TEST("test_d_string_find_last_matching_predicate_from_index", test_d_string_find_last_matching_predicate_from_index(););
    TEST("test_d_string_find_last_matching_predicate_from_end", test_d_string_find_last_matching_predicate_from_end(););
    
    TEST("test_d_string_find_first_not_matching_predicate_from_index", test_d_string_find_first_not_matching_predicate_from_index(););
    TEST("test_d_string_find_first_not_matching_predicate_from_start", test_d_string_find_first_not_matching_predicate_from_start(););
    
    TEST("test_d_string_find_last_not_matching_predicate_from_index", test_d_string_find_last_not_matching_predicate_from_index(););
    TEST("test_d_string_find_last_not_matching_predicate_from_end", test_d_string_find_last_not_matching_predicate_from_end(););
    
    TEST("test_d_string_trim_left_by_char_in_place", test_d_string_trim_left_by_char_in_place(););
    TEST("test_d_string_trim_left_by_char_new", test_d_string_trim_left_by_char_new(););

    TEST("test_d_string_trim_left_by_predicate_in_place", test_d_string_trim_left_by_predicate_in_place(););
    TEST("test_d_string_trim_left_by_predicate_new", test_d_string_trim_left_by_predicate_new(););
    
    TEST("test_d_string_trim_right_by_char_in_place", test_d_string_trim_right_by_char_in_place(););
    TEST("test_d_string_trim_right_by_char_new", test_d_string_trim_right_by_char_new(););
    
    TEST("test_d_string_trim_right_by_predicate_in_place", test_d_string_trim_right_by_predicate_in_place(););
    TEST("test_d_string_trim_right_by_predicate_new", test_d_string_trim_right_by_predicate_new(););

    TEST("test_d_string_split_by_char", test_d_string_split_by_char(););
    TEST("test_d_string_split_by_char_of_str", test_d_string_split_by_char_of_str(););
}