#include "container.h"

DResult container_next_pow2_checked(usize *pow2)
{
    usize tmp = *pow2;
    tmp <<= 1;
    if (tmp == 0)
        return D_ERR_OVERFLOW;
    *pow2 = tmp;
    return D_OK;
}