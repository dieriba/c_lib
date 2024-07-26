#ifndef __GENERAL_LIB__H
#define __GENERAL_LIB__H

#include <darray.h>

typedef bool(*DCompareFnc)(char);

DArray* d_split_string_by_predicate_fn(const char *string, DCompareFnc predicate);
DArray* d_split_string_by_str(const char* string, char*);
DArray* d_split_string_by_char(const char* string, char c);
DArray* d_split_string_by_pattern(const char*string, const char* pattern);

char    *d_itoa_i32(int32);
char    *d_itoa_u64(u64);
#endif