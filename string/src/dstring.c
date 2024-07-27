#include "dstring.h"
#include <dalloc.h>
#include <string.h>

#define CAPACITY 8

typedef struct _DRealString DRealString;

//Real String Interface
struct _DRealString {
    char    *string;
    usize     len;
    usize     capacity;
};


DString* d_string_new(void)
{
    DRealString* dstring = malloc(sizeof(DRealString));
    dstring -> len = 0;
    dstring -> capacity = CAPACITY;
    dstring -> string = malloc(sizeof(char) * (CAPACITY + 1));
    return (DString*)dstring;
}

DString* 	d_string_new_with_string(const char* str)
{
    DRealString* dstring = malloc(sizeof(DRealString));
    usize len = strlen(str);
    dstring -> len = len;
    dstring -> capacity = CAPACITY;
    dstring -> string = malloc(sizeof(char) * (len + 1));
    memcpy(dstring -> string, str, len + 1);
    return (DString*)dstring;
}

DString* 	d_string_new_with_reserve(usize reserve)
{
    DRealString* dstring = malloc(sizeof(DRealString));
    dstring -> len = 0;
    dstring -> capacity = reserve;
    dstring -> string = malloc(sizeof(char) * (reserve + 1));
    return (DString*)dstring;
}

usize		d_string_get_capacity(DString* dstring)
{
    DRealString* rdstring = (DRealString*)dstring;

    return rdstring -> capacity;
}

DString* 	d_string_resize(DString* dstring, usize len)
{
    DRealString* rdstring = (DRealString*)dstring;


}

DString* 	d_string_increase_capacity(DString* dstring, usize len)
{
    DRealString* rdstring = (DRealString*)dstring;
    usize capacity = rdstring -> capacity;
    capacity += len + 1; // adding 1 for the null bytes as each string will have a null bytes at the end so stdlib function can be used on them
    rdstring -> capacity = capacity + (capacity % 2 == 1);
    rdstring -> string = realloc(rdstring -> string, rdstring -> len + rdstring -> capacity);
    return dstring;
}

DString* 	d_string_push_char(DString* dstring, char c)
{
    DRealString* rdstring = (DRealString*)dstring;

    if (rdstring -> capacity == 0)
        d_string_increase_capacity(dstring, rdstring -> len * 2);
    rdstring->string[rdstring->len++] = c;
    --rdstring->capacity;
    return dstring;
}

DString* 	d_string_push_str(DString* dstring, const char *str_to_append)
{
    DRealString* rdstring = (DRealString*)dstring;
    usize len = strlen(str_to_append);

    if (rdstring -> capacity < len)
        d_string_increase_capacity(dstring, len * 2);
    memcpy(rdstring -> string + rdstring -> len, str_to_append, len + 1);
    rdstring -> len += len;
    return dstring;
}