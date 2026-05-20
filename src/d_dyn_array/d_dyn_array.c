#include "d_dyn_array.h"
#include "container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASSERT_FIRST_FIELD(DDynArray, array);

#define d_dyn_array_elt_len(arr, i) ((arr)->array.elem_size * (i))
#define d_dyn_array_elt_pos(arr, i) ((char *)(arr)->array.data + d_dyn_array_elt_len((arr), (i)))

DResult d_dyn_array_init(DDynArray *new_dyn_array, usize elem_size, usize reserved_elem, DestroyElemFn free_func, BufferOpts opts)
{
	if (new_dyn_array == NULL || elem_size == 0)
		return D_ERR_INVALID_ARG;

	new_dyn_array->free_func = free_func;
	return raw_buffer_init((RawBuffer *)new_dyn_array, elem_size, reserved_elem, opts);
}

DResult d_dyn_array_init_ptr_arr(DDynArray *new_dyn_array, usize reserved_elem, DestroyElemFn free_func, BufferOpts opts)
{
	return d_dyn_array_init(new_dyn_array, sizeof(void *), reserved_elem, free_func, opts);
}

DResult d_dyn_array_init_from(DDynArray *new_dyn_array, DDynArray *dyn_array)
{
	if (new_dyn_array == NULL || dyn_array == NULL)
		return D_ERR_INVALID_ARG;
	RawBuffer *raw_buffer = &dyn_array->array;
	DResult op_result = d_dyn_array_init(new_dyn_array, raw_buffer->elem_size, raw_buffer->capacity, dyn_array->free_func, raw_buffer->opts);
	if (op_result != D_OK)
		return op_result;
	if ((op_result = d_dyn_array_append(new_dyn_array, raw_buffer->data, raw_buffer->size)) != D_OK)
	{
		d_dyn_array_destroy(new_dyn_array);
		return op_result;
	}
	return D_OK;
}

DResult d_dyn_array_append(DDynArray *dyn_array, const void *data, usize nb_elem_to_copy)
{
	return raw_buffer_append_data((RawBuffer *)dyn_array, data, nb_elem_to_copy);
}

DResult d_dyn_array_push_back(DDynArray *dyn_array, const void *data)
{
	return raw_buffer_push((RawBuffer *)dyn_array, data);
}

DResult d_dyn_array_push_back_ptr(DDynArray *dyn_array, const void *data)
{
	return d_dyn_array_push_back(dyn_array, &data);
}

DResult d_dyn_array_get_elem_at(const DDynArray *dyn_array, usize index, void *out_elem)
{
	return raw_buffer_get_elem_at((RawBuffer *)dyn_array, index, out_elem);
}

DResult d_dyn_array_get_elem_addr_at(const DDynArray *dyn_array, usize index, void **out_elem)
{
	return raw_buffer_get_elem_addr((RawBuffer *)dyn_array, index, out_elem);
}

DResult d_dyn_array_get_size(const DDynArray *dyn_array, usize *size)
{
	return raw_buffer_get_size((RawBuffer *)dyn_array, size);
}

DResult d_dyn_array_get_capacity(const DDynArray *dyn_array, usize *capacity)
{
	return raw_buffer_get_capacity((RawBuffer *)dyn_array, capacity);
}

DResult d_dyn_array_remove_elem_fast(DDynArray *dyn_array, usize index, void *out_elem)
{
	if (dyn_array == NULL)
		return D_ERR_INVALID_ARG;
	DResult op_result;
	void *elem;
	if ((op_result = raw_buffer_get_elem_addr(&dyn_array->array, index, &elem)) != D_OK)
		return op_result;
	DestroyElemFn free_func = dyn_array->free_func;
	if (free_func && !out_elem)
		free_func(elem);
	return raw_buffer_swap_remove(&dyn_array->array, index, out_elem);
}

DResult d_dyn_array_remove_last_element(DDynArray *dyn_array, void *out_elem)
{
	if (dyn_array == NULL)
		return D_ERR_INVALID_ARG;
	usize size = dyn_array->array.size;
	if (size == 0)
		return D_OK;
	return d_dyn_array_remove_elem_fast(dyn_array, size - 1, out_elem);
}

static void destroy_elements(DDynArray *dyn_array)
{
	if (dyn_array == NULL)
		return;
	DestroyElemFn free_func = dyn_array->free_func;
	if (!free_func)
		return;
	usize elem_size = dyn_array->array.elem_size;
	usize arr_size = dyn_array->array.size;

	void *elem;
	for (size_t i = 0; i < arr_size; i++)
	{
		raw_buffer_get_elem_addr((RawBuffer *)dyn_array, i, &elem);
		free_func(elem);
		memset(elem, 0, elem_size);
	}
}

DResult d_dyn_array_clear_array(DDynArray *dyn_array)
{
	if (dyn_array == NULL)
		return D_ERR_INVALID_ARG;
	destroy_elements(dyn_array);
	dyn_array->array.size = 0;
	return D_OK;
}

void d_dyn_array_destroy(DDynArray *dyn_array)
{
	if (dyn_array == NULL)
		return;
	destroy_elements(dyn_array);
	raw_buffer_free((RawBuffer *)dyn_array);
	memset(dyn_array, 0, sizeof(DDynArray));
}

void d_dyn_array_dbg_print(DDynArray *dyn_array, FnElemRepr elem_repr)
{
	if (dyn_array == NULL)
	{
		printf("DDynArray: NULL\n");
		return;
	}
	usize size = dyn_array->array.size;
	printf("DDynArray { size: %zu, capacity: %zu, elems: [", size, dyn_array->array.capacity);
	for (usize i = 0; i < size; i++)
	{
		char *repr = elem_repr(d_dyn_array_elt_pos(dyn_array, i));
		if (i > 0)
			printf(", ");
		printf("%s", repr);
		free(repr);
	}
	printf("] }\n");
}