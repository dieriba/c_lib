#ifndef D_DYN_STRING_H
#define D_DYN_STRING_H
#include "raw_buffer.h"
#include "d_types.h"

typedef struct DDynString
{
    RawBuffer str;
} DDynString;

DResult d_dyn_string_init(DDynString *dyn_string);
DResult d_dyn_string_init_from_c_string(DDynString *dyn_string, const char *str);
DResult d_dyn_string_init_from_dstring(DDynString *dyn_string, DDynString *dstring);
DResult d_dyn_string_init_with_capacity(DDynString *dyn_string, usize reserve);
DResult d_dyn_string_init_with_sub_string(DDynString *dyn_string, const char *str, usize pos, usize size);

const char *d_dyn_string_get_string(const DDynString *dstring);
DResult d_dyn_string_get_char_at(const DDynString *dstring, usize i, void *out_elem);
DCompareResult d_dyn_string_compare(DDynString *d1, DDynString *d2);

DResult d_dyn_string_get_size(const DDynString *dstring, usize *size);
DResult d_dyn_string_get_capacity(const DDynString *dstring, usize *capacity);
DResult d_dyn_string_resize(DDynString *dstring, usize size, char c);

DResult d_dyn_string_push_char(DDynString *dstring, char c);
DResult d_dyn_string_push_str_with_len(DDynString *dstring, const char *str_to_append, usize size);
DResult d_dyn_string_push_c_str(DDynString *dstring, const char *str_to_append);
DResult d_dyn_string_merge(DDynString *dstring1, DDynString *dstring2);

DResult d_dyn_string_replace_from_str(DDynString *dstring, const char *str);
DResult d_dyn_string_replace_from_dstring(DDynString *dstring, const DDynString *to_copy);

DResult d_dyn_string_sub_string_in_place(DDynString *dstring, usize pos, usize size);

void d_dyn_string_destroy(DDynString *dstring);
#endif