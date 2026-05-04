#ifndef D_DYN_STRING_H
#define D_DYN_STRING_H

#include "d_types.h"

typedef struct _DynString DynString;

DynString *d_dyn_string_new(void);
DynString *d_dyn_string_new_from_c_string(const char *str);
DynString *d_dyn_string_new_from_dstring(DynString *dstring);
DynString *d_dyn_string_new_with_reserve(usize reserve);
DynString *d_dyn_string_new_with_sub_string(const char *str, usize pos, usize len);
void d_dyn_string_destroy(DynString **dstring);

const char const *d_dyn_string_get_string(DynString *dstring);
char *d_dyn_string_get_char_at(DynString *dstring, usize i);

DResult d_dyn_string_resize(DynString *dstring, usize len, char c);

DResult d_dyn_string_push_char(DynString *dstring, char c);
DResult d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len);
DResult d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append);
DResult d_dyn_string_merge(DynString *dstring1, DynString *dstring2);

DResult d_dyn_string_replace_from_str(DynString *dstring, const char *str);
DResult d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy);

DResult d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len);

#endif