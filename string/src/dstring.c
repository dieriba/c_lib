#include <dstring.h>

typedef struct _DRealString DRealString;

//Real String Interface
struct _DRealString {
    char    *string;
    usize     len;
    usize     capacity;
};

