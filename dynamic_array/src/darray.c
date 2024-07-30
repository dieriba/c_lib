#include <darray.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 4

typedef struct _DRealArray DRealArray;

//REAL D_ARRAY STRUCTURE ALLOCATED
struct _DRealArray {
	void  *data;
	usize   len;
	usize   capacity;
	usize   elem_size;
	bool  clear: 1;
};

#define d_array_elt_len(array,i) ((array)->elem_size * (i))
//UTILITY MACRO TO GET TO WHICH OFFSET FROM THE START OF THE ARRAY A D_ARRAY ELEMENT IS
#define d_array_elt_pos(array,i) ((array)->data + d_array_elt_len((array),(i)))

static bool d_array_try_expand(DRealArray *array, usize len);

DArray  *d_array_new				(bool	clear,		usize elem_size, usize reserved_elem)
{
	DRealArray  *array = malloc(sizeof(DRealArray) * 1);
	if (array == NULL)
		return (NULL);
	array -> capacity = ((reserved_elem > 0) * reserved_elem) + ((reserved_elem == 0) * (usize)CAPACITY);
	array -> clear = clear;
	array -> elem_size = elem_size;
	array -> data = malloc(elem_size * array -> capacity);
	array -> len = 0;
	if (array -> data == NULL)
	{
		free(array);
		return NULL;
	}
	if (clear == true)
		memset(array->data, 0, elem_size * reserved_elem);
	return (DArray*) array;
}

DArray  *d_array_append_vals		(DArray *arr, 	const void *data,		usize len)
{
	DRealArray  *array = (DRealArray*) arr;
	if (array -> capacity < len && d_array_try_expand(array, len) == false)
		return NULL;
	memcpy(d_array_elt_pos(array,array->len), data, d_array_elt_len(array, len));
	array->capacity -= len;
	array->len += len;
	return arr;
}

DArray	*d_array_copy(DArray* array)
{
	DRealArray* rarray = (DRealArray*)array;
	DArray* new_array = d_array_new(rarray -> clear, rarray -> elem_size, rarray -> len * 2);
	d_array_append_vals(new_array, rarray -> data, rarray -> len);
	return new_array;
}

usize d_array_get_capacity(DArray* array)
{
	DRealArray* rarray = (DRealArray*) array;
	return rarray -> capacity;
}

DArray* d_array_modify_capacity(DArray* array, usize new_capacity)
{
	DRealArray* rarray = (DRealArray*) array;
	if (new_capacity == rarray -> capacity)
		return array;
	rarray -> capacity = new_capacity;
	array -> data = reallocarray(array -> data, rarray -> len + new_capacity, rarray -> elem_size);
	if (array -> data == NULL)
		return NULL;
	return array;
}

DArray  *d_array_pop_back	(DArray	*arr)
{
	DRealArray* array = (DRealArray*)arr;
	array -> len -= (array -> len > 0);
	return arr;
}

DArray  *d_array_remove_index_fast	(DArray	*arr, 	usize	index)
{
	DRealArray* array = (DRealArray*)arr;
	if (index >= array -> len)
		return NULL;
	memcpy(d_array_elt_pos(array, index), d_array_elt_pos(array, array -> len - 1), d_array_elt_len(array, 1));
	array -> capacity++;
	array -> len--;
	return arr;
}

void    d_array_destroy		(DArray** arr)
{
	DRealArray*	array = (DRealArray*)(*arr);
	free(array->data);
	free(array);
	*arr = NULL;
}

DArray  *d_array_clear_array		(DArray* arr)
{
	DRealArray* array = (DRealArray*)arr;
	array -> capacity = array -> len;
	array->len = 0;
	return arr;
}


bool d_array_try_expand(DRealArray *arr, usize len)
{
	DRealArray* array = (DRealArray*)arr;
	usize arr_len = array -> len;
	usize new_arr_size = ((len == 1) * arr_len * 2) + ((len > 1) * (len + (arr_len * 2)));
	new_arr_size += (new_arr_size % 2) == 1;
	array->capacity = new_arr_size - arr_len;
	array -> data = reallocarray(array->data, new_arr_size, array->elem_size);
	if (array -> data != NULL && array -> clear == true)
		memset(d_array_elt_pos(array, arr_len + len), 0, d_array_elt_len(array, new_arr_size - (arr_len + len)));
	return array -> data != NULL;
}

/*-------------------------------------------------DPointerArray-------------------------------------------------*/

typedef struct _DRealPointerArray  DRealPointerArray;
typedef void(*DestroyElemFunc)(void*);


struct _DRealPointerArray
{
  	void**       	pdata;
  	usize         	len;
	usize				capacity;
  	u8          	null_terminated : 1; /* always either 0 or 1, so it can be added to array lengths */
	DestroyElemFunc	free_func; /*if not null will be used on each element when de-allocating or clearing the array*/
};

static bool d_pointer_array_try_expand(DPointerArray *array, usize len);

DPointerArray  *d_pointer_array_new	(usize reserved_elem, bool null_terminated)
{
	DRealPointerArray  *array = malloc(sizeof(DRealPointerArray) * 1);
	if (array == NULL)
		return (NULL);
	array -> null_terminated = null_terminated;
	array -> capacity = (((reserved_elem > 0) * reserved_elem) + ((reserved_elem == 0) + array -> null_terminated) * (usize)CAPACITY);
	array -> pdata = malloc(sizeof(void*) * array -> capacity);
	array -> len = 0;
	if (array -> pdata == NULL)
	{
		free(array);
		return NULL;
	}
	if (array -> null_terminated)
		array->pdata[0] = NULL;
	return (DPointerArray*) array;
}

DPointerArray  *d_pointer_array_push_back		(DPointerArray *arr, 	const void *data)
{
	DRealPointerArray* array = (DRealPointerArray*)arr;
	usize	null_terminated = (usize)array -> null_terminated;
	if (array -> capacity - null_terminated == 0 && d_pointer_array_try_expand(arr, 1) == false)
		return NULL;
	array -> pdata[array -> len++] = (void*)data;
	if (null_terminated)
		array -> pdata[array -> len] = NULL;
	return arr;
}

DPointerArray  *d_pointer_array_remove_index_fast	(DPointerArray	*arr, 	usize	index)
{
	DRealPointerArray* array = (DRealPointerArray*)arr;
	if (index >= array -> len)
		return NULL;
	if (array -> free_func)
		array -> free_func(array -> pdata[index]);
	array -> pdata[index] = array -> pdata[array -> len - 1 - array -> null_terminated];
	array -> capacity++;
	array -> len--;
	return arr;
}

void    d_pointer_array_destroy		(DPointerArray** arr)
{
	DRealPointerArray*	array = (DRealPointerArray*)(*arr);
	DestroyElemFunc	free_func = array -> free_func;

	if (free_func)
	{
		size_t	i = 0;
		for (; i < array -> len; i++)
		{
			free_func(array->pdata[i]);
			array->pdata[i] = NULL;
		}
		memset(array + i, 0, sizeof(DRealPointerArray));
	}
	free(array->pdata);
	free(array);
	*arr = NULL;
}

DPointerArray  *d_pointer_array_clear_array		(DPointerArray* arr)
{
	DRealPointerArray* array = (DRealPointerArray*)arr;
	DestroyElemFunc	free_func = array -> free_func;

	if (free_func)
	{
		for (size_t i = 0; i < array -> len; i++)
		{
			free_func(array->pdata[i]);
			array->pdata[i] = NULL;
		}
	}

	array->len = 0;
	return arr;
}


bool d_pointer_array_try_expand(DPointerArray *arr, usize len)
{
	DRealPointerArray* array = (DRealPointerArray*)arr;
	usize arr_len = array -> len;
	usize new_arr_size = ((len == 1) * arr_len * 2) + (len > 1) * (len + (arr_len * 2)) + array -> null_terminated;
	new_arr_size += (new_arr_size % 2) == 1;
	array->capacity = new_arr_size - arr_len;
	array -> pdata = reallocarray(array->pdata, new_arr_size, sizeof(void*));
	return array -> pdata != NULL;
}