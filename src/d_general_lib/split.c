#include "d_general_lib.h"
#include "d_string_view.h"

DynArray *d_split_string_by_char_of_str(const char *string, char *delims, BufferOpts opts)
{
    return d_string_view_split_by_char_of_str(d_string_view_from_c_string(string), opts, delims);
}

DynArray *d_split_string_by_char(const char *string, char c, BufferOpts opts)
{
    return d_string_view_split_by_char(d_string_view_from_c_string(string), opts, c);
}
