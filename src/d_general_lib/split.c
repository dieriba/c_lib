#include "d_general_lib.h"

/*DPointerArray* d_split_string_by_predicate_fn(const char *string, DCompareFnc predicate)
{

}*/

DynArray *d_split_string_by_char_of_str(const char *string, char *delims)
{
    DynString *dstring = d_dyn_string_new_from_c_string(string);
    if (dstring == NULL)
        return NULL;
    DynArray *arr = d_dyn_string_split_by_char_of_str(dstring, delims);
    d_dyn_string_destroy(&dstring);
    return arr;
}

DynArray *d_split_string_by_char(const char *string, char c)
{
    DynString *dstring = d_dyn_string_new_from_c_string(string);
    if (dstring == NULL)
        return NULL;
    DynArray *arr = d_dyn_string_split_by_char(dstring, c);
    d_dyn_string_destroy(&dstring);
    return arr;
}

/*DPointerArray* d_split_string_by_pattern(const char*string, const char* pattern)
{

}*/