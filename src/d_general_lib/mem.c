#include "d_general_lib.h"

void *memfill(void *dst, void *filler_elem, usize filler_size, usize nb_slot_to_fill)
{
    if (dst == NULL)
        return NULL;

    char *base = dst;

    usize filled_slot = 0;

    while (filled_slot < nb_slot_to_fill)
    {

    }

    return dst;
}