#ifndef __D_ARRAY_H
#define __D_ARRAY_H

#include "Dtypes.h"

typedef struct _DArray		DArray;

struct _DArray {
	void        *data;
	u_int64_t   len;
};

#define d_array_append_val(a,v)	  d_array_append_vals (a, &(v), 1)
#define d_array_prepend_val(a,v)  d_array_prepend_vals (a, &(v), 1)
#define d_array_insert_val_at_index(a,i,v) d_array_insert_vals (a, i, &(v), 1)
#define d_array_get_val_by_index(a,t,i)      (((t*) (void *) (a)->data) [(i)])


DArray  *d_array_new				(bool	clear,		u_int8_t elem_size);
DArray  *d_array_append_vals		(DArray *array, 	const void *data,		u_int64_t len);
DArray  *d_array_prepend_vals		(DArray *array, 	const void *data,		u_int64_t len);
DArray  *d_array_insert_vals		(DArray *arrary,	u_int64_t	index,	const void *data, u_int64_t len);
DArray  *g_array_remove_index       (DArray	*array,		u_int64_t	index_);
DArray  *g_array_remove_index_fast  (DArray	*array, 	u_int64_t	index_);
DArray  *g_array_remove_range       (DArray	*array, 	u_int64_t	index_, u_int64_t	length);

#endif