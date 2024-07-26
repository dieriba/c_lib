#ifndef __D_STRING__H__
#define __D_STRING__H__

#include <dtypes.h>

typedef struct _DString DString;

struct _DString {
    char    *string;
    u64     len;
};


DString* dstring_new(void);
DString* dstring_new_with_string(const char* str);
DString* dstring_new_with_reserve(u64 reserve);
DString* dstring_new_with_reserve_and_string(const char* str, u64 reserve);

DString* d_string_push_char(char c);
DString* d_string_push_str(char *str);
DString* d_string_resize(u64 len);


#endif