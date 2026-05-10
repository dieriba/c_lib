#include "d_general_lib.h"
#include "d_string_view.h"

DResult d_split_string_by_char_of_str(DDynArray *new_dyn_array, DStringView view, DStringView set, BufferOpts opts)
{
    if (new_dyn_array == NULL)
        return D_ERR_INVALID_ARG;
    return d_string_view_split_by_char_of_str(new_dyn_array, view, opts, set);
}

DResult d_split_string_by_char(DDynArray *new_dyn_array, DStringView view, char c, BufferOpts opts)
{
    if (new_dyn_array == NULL)
        return D_ERR_INVALID_ARG;
    return d_string_view_split_by_char(new_dyn_array, view, opts, c);
}
