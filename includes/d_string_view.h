#ifndef D_STRING_VIEW_H
#define D_STRING_VIEW_H

#include "d_types.h"
#include "d_dyn_string.h"
#include "d_dyn_array.h"

typedef struct DStringView
{
    const char *data;
    const usize size;
} DStringView;

typedef bool (*match)(char c);

#define d_string_view_new_from_str(str)            \
    do                                             \
    {                                              \
        d_string_view_from_parts(str, sizeof(str)) \
    } while (0)

DStringView d_string_view_from_parts(const char *data, usize size);
DStringView d_string_view_from_c_string(const char *c_str);
DStringView d_string_view_from_dyn_string(const DDynString *dstring);

bool d_string_view_is_empty(DStringView view);
usize d_string_view_len(DStringView view);
const char *d_string_view_data(DStringView view);

char d_string_view_get_char_at(DStringView view, usize index);

DStringView d_string_view_subview(DStringView view, usize pos, usize size);

int d_string_view_compare(DStringView view1, DStringView view2);
int d_string_view_compare_against_c_string(DStringView view, const char *c_str);
bool d_string_view_equals(DStringView view1, DStringView view2);
bool d_string_view_equals_c_string(DStringView view, const char *c_str);

bool d_string_view_starts_with_char(DStringView view, char c);
bool d_string_view_ends_with_char(DStringView view, char c);
bool d_string_view_starts_with_view(DStringView view, DStringView prefix);
bool d_string_view_ends_with_view(DStringView view, DStringView suffix);
bool d_string_view_starts_with_c_string(DStringView view, const char *prefix);
bool d_string_view_ends_with_c_string(DStringView view, const char *suffix);

usize d_string_view_find_first_matching_char_from_index(DStringView view, char c, usize pos);
usize d_string_view_find_first_matching_char_from_start(DStringView view, char c);
usize d_string_view_find_first_not_matching_char_from_index(DStringView view, char c, usize pos);
usize d_string_view_find_first_not_matching_char_from_start(DStringView view, char c);

usize d_string_view_find_last_matching_char_from_index(DStringView view, char c, usize pos);
usize d_string_view_find_last_matching_char_from_end(DStringView view, char c);
usize d_string_view_find_last_not_matching_char_from_index(DStringView view, char c, usize pos);
usize d_string_view_find_last_not_matching_char_from_end(DStringView view, char c);

usize d_string_view_find_first_matching_view_from_index(DStringView view, DStringView to_find, usize pos);
usize d_string_view_find_first_matching_view_from_start(DStringView view, DStringView to_find);
usize d_string_view_find_first_matching_c_string_from_index(DStringView view, const char *str, usize pos);
usize d_string_view_find_first_matching_c_string_from_start(DStringView view, const char *str);

usize d_string_view_find_last_matching_view_from_index(DStringView view, DStringView to_find, usize pos);
usize d_string_view_find_last_matching_view_from_end(DStringView view, DStringView to_find);
usize d_string_view_find_last_matching_c_string_from_index(DStringView view, const char *str, usize pos);
usize d_string_view_find_last_matching_c_string_from_end(DStringView view, const char *str);

usize d_string_view_find_first_char_in_set_from_index(DStringView view, const char *set, usize pos);
usize d_string_view_find_first_char_in_set_from_start(DStringView view, const char *set);
usize d_string_view_find_first_char_not_in_set_from_index(DStringView view, const char *set, usize pos);
usize d_string_view_find_first_char_not_in_set_from_start(DStringView view, const char *set);

usize d_string_view_find_last_char_in_set_from_index(DStringView view, const char *set, usize pos);
usize d_string_view_find_last_char_in_set_from_end(DStringView view, const char *set);
usize d_string_view_find_last_char_not_in_set_from_index(DStringView view, const char *set, usize pos);
usize d_string_view_find_last_char_not_in_set_from_end(DStringView view, const char *set);

usize d_string_view_find_first_matching_predicate_from_index(DStringView view, match fn, usize pos);
usize d_string_view_find_first_matching_predicate_from_start(DStringView view, match fn);
usize d_string_view_find_first_not_matching_predicate_from_index(DStringView view, match fn, usize pos);
usize d_string_view_find_first_not_matching_predicate_from_start(DStringView view, match fn);

usize d_string_view_find_last_matching_predicate_from_index(DStringView view, match fn, usize pos);
usize d_string_view_find_last_matching_predicate_from_end(DStringView view, match fn);
usize d_string_view_find_last_not_matching_predicate_from_index(DStringView view, match fn, usize pos);
usize d_string_view_find_last_not_matching_predicate_from_end(DStringView view, match fn);

DStringView d_string_view_trim_left_by_char(DStringView view, char c);
DStringView d_string_view_trim_right_by_char(DStringView view, char c);
DStringView d_string_view_trim_left_by_predicate(DStringView view, match fn);
DStringView d_string_view_trim_right_by_predicate(DStringView view, match fn);

DResult d_string_view_split_by_char(DDynArray **new_dyn_array, DStringView view, BufferOpts opts, char c);
DResult d_string_view_split_by_char_of_str(DDynArray **new_dyn_array, DStringView view, BufferOpts opts, char *str);

DResult d_dyn_string_new_from_string_view(DDynString **new_dyn_string, DStringView view);

#endif