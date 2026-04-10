#ifndef D_DYN_STRING_H
#define D_DYN_STRING_H

#include "d_types.h"

typedef struct _DynString DynString;

/*
** Constructors / destruction
*/
DynString *d_dyn_string_new(void);
DynString *d_dyn_string_new_from_c_string(const char *str);
DynString *d_dyn_string_new_from_dstring(DynString *dstring);
DynString *d_dyn_string_new_with_reserve(usize reserve);
DynString *d_dyn_string_new_with_sub_string(const char *str, usize pos, usize len);
void d_dyn_string_destroy(DynString **dstring);

/*
** Basic accessors
*/
usize d_dyn_string_get_capacity(DynString *dstring);
char* d_dyn_string_get_data(DynString* dstring);
usize d_dyn_string_get_len(DynString* dstring);
char d_dyn_string_get_char_at(DynString *dstring, usize i);

/*
** Capacity management
*/
DynString *d_dyn_string_resize(DynString *dstring, usize len);
DynString *d_dyn_string_increase_capacity_if_needed(DynString *dstring, usize to_copy);

/*
** Mutation
*/
DynString *d_dyn_string_push_char(DynString *dstring, char c);
DynString *d_dyn_string_push_str_with_len(DynString *dstring, const char *str_to_append, usize len);
DynString *d_dyn_string_push_c_str(DynString *dstring, const char *str_to_append);
DynString *d_dyn_string_merge(DynString *dstring1, DynString *dstring2);

DynString *d_dyn_string_replace_from_str(DynString *dstring, const char *str);
DynString *d_dyn_string_replace_from_dstring(DynString *dstring, const DynString *to_copy);

/*
** Owning substring helpers
*/
char *d_dyn_string_substr(DynString *dstring, usize pos, usize len);
DynString *d_dyn_string_sub_string_in_place(DynString *dstring, usize pos, usize len);

#endif