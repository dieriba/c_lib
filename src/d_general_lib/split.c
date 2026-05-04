#include "d_general_lib.h"
#include "d_string_view.h"

DResult d_split_string_by_char_of_str(DDynArray **new_dyn_array, const char *string, char *delims, BufferOpts opts)
{
    return d_string_view_split_by_char_of_str(new_dyn_array, d_string_view_from_c_string(string), opts, delims);
}

DResult d_split_string_by_char(DDynArray **new_dyn_array, const char *string, char c, BufferOpts opts)
{
    return d_string_view_split_by_char(new_dyn_array, d_string_view_from_c_string(string), opts, c);
}
