#include "d_dyn_array.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 0x10
#define DYN_ARRAY_GROWTH_POLICY 2
#define DYN_ARRAY_GROWTH_LIMIT (MAX_SIZE_T_VALUE / DYN_ARRAY_GROWTH_POLICY)

struct _DynArray
{
	void *data;
	usize len;
	usize capacity;
	usize elem_size;
	bool clear : 1;
};

#define d_dyn_array_elt_len(array, i) ((array)->elem_size * (i))
#define d_dyn_array_elt_pos(array, i) ((char *)(array)->data + d_dyn_array_elt_len((array), (i)))

DynArray *d_dyn_array_new(usize elem_size, usize reserved_elem, bool clear)
{
	if (elem_size == 0)
		return NULL;
	DynArray *dyn_array = malloc(sizeof(DynArray));
	if (dyn_array == NULL)
		return (NULL);
	dyn_array->capacity = reserved_elem == 0 ? DEFAULT_CAPACITY : reserved_elem;
	dyn_array->clear = clear;
	dyn_array->elem_size = elem_size;
	dyn_array->data = malloc(elem_size * dyn_array->capacity);
	if (dyn_array->data == NULL)
	{
		free(dyn_array);
		return NULL;
	}
	dyn_array->len = 0;
	if (clear == true)
		memset(dyn_array->data, 0, elem_size * dyn_array->capacity);
	return dyn_array;
}

static DynArray *increase_capacity_if_needed(DynArray *darray, usize to_copy)
{
	if (darray == NULL)
		return NULL;

	if (MAX_SIZE_T_VALUE - darray->len < to_copy)
		return NULL;

	usize needed = darray->len + to_copy;
	if (needed <= darray->capacity)
		return darray;

	if (d_dyn_array_elt_len(darray, needed) > DYN_ARRAY_GROWTH_LIMIT)
		return NULL;

	usize new_capacity = needed * DYN_ARRAY_GROWTH_POLICY;

	char *tmp = realloc(darray->data, new_capacity * darray->elem_size);
	if (tmp == NULL)
		return NULL;

	darray->data = tmp;
	darray->capacity = new_capacity;
	return darray;
}

DynArray *d_dyn_array_append(DynArray *dyn_array, const void *data, usize len)
{
	if (len != 0 && data == NULL)
		return NULL;
	if (increase_capacity_if_needed(dyn_array, len) == NULL)
		return NULL;
	memcpy(d_dyn_array_elt_pos(dyn_array, dyn_array->len), data, d_dyn_array_elt_len(dyn_array, len));
	dyn_array->len += len;
	return dyn_array;
}

DynArray *d_dyn_array_copy(DynArray *dyn_array)
{
	DynArray *new_array = d_dyn_array_new(dyn_array->elem_size, dyn_array->capacity, dyn_array->clear);
	d_dyn_array_append_vals(new_array, dyn_array->data, dyn_array->len);
	return new_array;
}

usize d_dyn_array_get_capacity(DynArray *dyn_array)
{
	return dyn_array->capacity;
}

DynArray *d_dyn_array_modify_capacity(DynArray *dyn_array, usize new_capacity)
{
	if (new_capacity <= dyn_array->capacity)
		return dyn_array;
	void *tmp = realloc(dyn_array->data, new_capacity * dyn_array->elem_size);
	if (tmp == NULL)
		return NULL;
	dyn_array->data = tmp;
	dyn_array->capacity = new_capacity;
	return dyn_array;
}

DynArray *d_dyn_array_remove_elem_fast(DynArray *dyn_array, usize index)
{
	if (index >= dyn_array->len)
		return NULL;
	memcpy(d_dyn_array_elt_pos(dyn_array, index), d_dyn_array_elt_pos(dyn_array, dyn_array->len - 1), d_dyn_array_elt_len(dyn_array, 1));
	dyn_array->len--;
	return dyn_array;
}

DynArray *d_dyn_array_clear_array(DynArray *dyn_array)
{
	dyn_array->len = 0;
	return dyn_array;
}

void d_dyn_array_destroy(DynArray **array)
{
	if (array == NULL || *array == NULL)
		return;
	DynArray *dyn_array = *array;
	free(dyn_array->data);
	free(dyn_array);
	*array = NULL;
}

/*-------------------------------------------------DPointerArray-------------------------------------------------*/

struct _DPointerArray
{
	void **pdata;
	usize len;
	usize capacity;
	u8 null_terminated : 1;	   /* always either 0 or 1, so it can be added to array lengths */
	DestroyElemFunc free_func; /*if not null will be used on each element when de-allocating or clearing the array*/
};

DPointerArray *d_pointer_array_new(usize reserved_elem, bool null_terminated, DestroyElemFunc free_func)
{
	DPointerArray *array = malloc(sizeof(DPointerArray) * 1);
	if (array == NULL)
		return (NULL);
	array->capacity = (reserved_elem == 0 ? DEFAULT_CAPACITY : reserved_elem) + (usize)(null_terminated == true);
	array->pdata = malloc(sizeof(void *) * array->capacity);
	if (array->pdata == NULL)
	{
		free(array);
		return NULL;
	}
	array->free_func = free_func;
	array->null_terminated = (u8)null_terminated;
	array->len = 0;
	if (array->null_terminated)
		array->pdata[0] = NULL;
	return (DPointerArray *)array;
}

DPointerArray *d_pointer_array_append_vals(DPointerArray *arr, const void **data, usize len)
{
	DPointerArray *array = (DPointerArray *)arr;
	if (array->capacity < len && d_pointer_array_try_expand(arr, len) == false)
		return NULL;
	memcpy(array->pdata + array->len, data, sizeof(void *) * len);
	array->capacity -= len;
	array->len += len;
	if (array->null_terminated)
		array->pdata[array->len] = NULL;
	return arr;
}

usize d_pointer_array_get_capacity(DPointerArray *array)
{
	return array->capacity;
}

DPointerArray *d_pointer_array_modify_capacity(DPointerArray *array, usize new_capacity)
{
	if (new_capacity <= array->capacity)
		return array;
	void **tmp = realloc(array->pdata, new_capacity * sizeof(void *));
	if (tmp == NULL)
		return NULL;
	array->pdata = tmp;
	array->capacity = new_capacity;
	return array;
}

DPointerArray *d_pointer_array_push_back(DPointerArray *arr, const void *data)
{
	DPointerArray *array = (DPointerArray *)arr;
	usize null_terminated = (usize)array->null_terminated;
	// increase_capacity_if_needed
	array->pdata[array->len++] = (void *)data;
	--array->capacity;
	if (null_terminated)
		array->pdata[array->len] = NULL;
	return arr;
}

DPointerArray *d_pointer_array_remove_index_fast(DPointerArray *arr, usize index)
{
	DPointerArray *array = (DPointerArray *)arr;
	DestroyElemFunc free_func = array->free_func;
	if (index >= array->len)
		return NULL;
	void *tmp = array->pdata[index];
	array->pdata[index] = array->pdata[array->len - 1];
	if (free_func != NULL)
		free_func(tmp);
	array->pdata[array->len - 1] = NULL;
	--array->len;
	return arr;
}

static void destroy_elements(DPointerArray *array)
{
	DestroyElemFunc free_func = array->free_func;

	if (free_func)
	{
		for (size_t i = 0; i < array->len; i++)
		{
			free_func(array->pdata[i]);
			array->pdata[i] = 0;
		}
	}
}

void d_pointer_array_destroy(DPointerArray **arr)
{
	DPointerArray *array = (DPointerArray *)(*arr);
	destroy_elements(array);
	free(array->pdata);
	free(array);
	*arr = NULL;
}

DPointerArray *d_pointer_array_clear_array(DPointerArray *arr)
{
	DPointerArray *array = (DPointerArray *)arr;
	destroy_elements(array);
	array->len = 0;
	return arr;
}
