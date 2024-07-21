#include "darray.h"
#include "dalloc.h"
#include <string.h>

#define CAPACITY 4 

typedef struct _DRealArray DRealArray;

struct _DRealArray {
	void        *data;
	u_int64_t   len;
	u_int64_t   capacity;
	u_int64_t    elem_size;
	bool        clear: 1;
};

#define d_array_elt_len(array,i) ((array)->elem_size * (i))
#define d_array_elt_pos(array,i) ((array)->data + d_array_elt_len((array),(i)))

static bool d_array_try_expand(DRealArray *array, u_int64_t len);

DArray  *d_array_new				(bool	clear,		u_int64_t elem_size, u_int64_t reserved_elem)
{
	DRealArray  *array = d_alloc_new(DRealArray, 1);
	if (array == NULL)
		return (NULL);
	array -> capacity = ((reserved_elem > 0) * reserved_elem) + ((reserved_elem == 0) * (u_int64_t)CAPACITY);
	array -> clear = clear;
	array -> elem_size = elem_size;
	array -> data = malloc(elem_size * array -> capacity);
	array -> len = 0;
	if (array -> data == NULL)
	{
		d_alloc_free(array);
		return NULL;
	}
	if (clear == true)
		memset(array->data, 0, elem_size * reserved_elem);
	return (DArray*) array;
}

DArray  *d_array_append_vals		(DArray *arr, 	const void *data,		u_int64_t len)
{
	DRealArray  *array = (DRealArray*) arr;
	if (array -> capacity < len && d_array_try_expand(array, len) == false)
		return NULL;
	memcpy(d_array_elt_pos(array,array->len), data, d_array_elt_len(array, len));
	array->capacity -= len;
	array->len += len;
	return arr;
}

DArray  *d_array_remove_index_fast	(DArray	*arr, 	u_int64_t	index)
{
	DRealArray* array = (DRealArray*)arr;
	if (index >= array -> len)
		return NULL;
	memcpy(d_array_elt_pos(array, index), d_array_elt_pos(array, array -> len - 1), d_array_elt_len(array, 1));
	array -> capacity++;
	array -> len--;
	return arr;
}

void    d_array_destroy_d_array		(DArray** arr)
{
	DRealArray*	array = (DRealArray*)(*arr);
	free(array->data);
	memset(array, 0, sizeof(DRealArray));
	free(array);
	*arr = NULL;
}

DArray  *d_array_clear_array		(DArray* arr)
{
	DRealArray* array = (DRealArray*)arr;
	array->capacity = CAPACITY;
	array->len = 0;
	return arr;
}


bool d_array_try_expand(DRealArray *arr, u_int64_t len)
{
	DRealArray* array = (DRealArray*)arr;
	u_int64_t arr_len = array -> len;
	u_int64_t new_arr_size = ((len == 1) * arr_len * 2) + (len > 1 * ((((len % 2) == 0) + len + arr_len) * 2));
	array->capacity = new_arr_size - arr_len;
	array -> data = reallocarray(array->data, new_arr_size, array->elem_size);
	if (array -> data != NULL && array -> clear == true)
		memset(d_array_elt_pos(array, arr_len + len), 0, d_array_elt_len(array, new_arr_size - (arr_len + len)));
	return array -> data != NULL;
}
