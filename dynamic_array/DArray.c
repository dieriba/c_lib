#include "DArray.h"

typedef struct _DRealArray DRealArray;

struct _DRealArray {
    void        *data;
    u_int64_t   len;
    u_int64_t   capacity;
    u_int8_t    elem_size;
    bool        clear: 1;
};

static bool d_array_try_expand(DRealArray *array);

bool d_array_try_expand(DRealArray *array)
{

}

DArray  *d_array_new				(bool	clear,		u_int8_t elem_size)
{

}

DArray  *d_array_append_vals		(DArray *array, 	const void *data,		u_int64_t len)
{

}

DArray  *d_array_prepend_vals		(DArray *array, 	const void *data,		u_int64_t len)
{

}

DArray  *d_array_insert_vals		(DArray *arrary,	u_int64_t	index,	const void *data, u_int64_t len)
{

}

DArray  *g_array_remove_index       (DArray	*array,		u_int64_t	index_)
{

}

DArray  *g_array_remove_index_fast  (DArray	*array, 	u_int64_t	index_)
{

}

DArray  *g_array_remove_range       (DArray	*array, 	u_int64_t	index_, u_int64_t	length)
{

}
