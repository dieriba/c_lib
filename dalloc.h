#ifndef __D_ALLOC__H
#define __D_ALLOC__H

#include <stdlib.h>

#define d_alloc_new(type, len) (malloc(sizeof(type) * len))
#define d_alloc_free(ptr) (free(ptr))
#endif 