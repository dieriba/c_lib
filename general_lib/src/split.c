#include <general_lib.h>


/*DPointerArray* d_split_string_by_predicate_fn(const char *string, DCompareFnc predicate)
{

}*/

DPointerArray* d_split_string_by_char_of_str(const char* string, const char* delims)
{
    DString* dstring = d_string_new_from_c_string(string);
    if (dstring == NULL)
        return NULL;
    return (free(dstring), d_string_split_by_char_of_str(dstring, delims));
}

DPointerArray* d_split_string_by_char(const char* string, char c)
{
    DString* dstring = d_string_new_from_c_string(string);
    if (dstring == NULL)
        return NULL;
    return (free(dstring), d_string_split_by_char(dstring, c));
}

/*DPointerArray* d_split_string_by_pattern(const char*string, const char* pattern)
{

}*/