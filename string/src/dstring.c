#include <dstring.h>

typedef struct _DRealString DRealString;

//Real String Interface
struct _DRealString {
    char    *string;
    u64     len;
    u64     capacity;
};

