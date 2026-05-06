#include "d_general_lib.h"

void *memfill(void *dst, void *filler_elem, usize filler_elem_size, usize nb_slot_to_fill)
{
    if (dst == NULL || filler_elem == NULL || filler_elem_size == 0)
        return NULL;
    else if (nb_slot_to_fill == 0)
        return dst;
    memcpy(dst, filler_elem, filler_elem_size);
    usize filled_slot = 1;
    char *base = (char *)dst;
    while (filled_slot < nb_slot_to_fill)
    {
        usize doubled_filled_slot = filled_slot << 1;
        usize size_to_cpy = doubled_filled_slot > nb_slot_to_fill ? nb_slot_to_fill - filled_slot : filled_slot;
        memcpy(base + (filled_slot * filler_elem_size), base, size_to_cpy * filler_elem_size);
        filled_slot = doubled_filled_slot;
    }

    return dst;
}