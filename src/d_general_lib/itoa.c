#include "d_general_lib.h"

#define fill_buff_with_num_digits(buffer, nbr, size, stop) \
    buffer[size--] = 0;                                    \
    for (int32 i = size; i >= stop; --i)                   \
    {                                                      \
        buffer[i] = (nbr % 10) + 48;                       \
        nbr /= 10;                                         \
    }

#define get_size_nb(nbr, size) \
    size = (nbr <= 0);         \
                               \
    while (nbr != 0)           \
    {                          \
        nbr /= 10;             \
        ++size;                \
    }

static void num_as_str(long long nbr, int32 size, char *buffer)
{
    int32 is_neg = nbr < 0;
    usize cpy;
    if (is_neg)
    {
        buffer[0] = '-';
        cpy = -nbr;
    }
    else
        cpy = nbr;

    fill_buff_with_num_digits(buffer, cpy, size, is_neg);
}

char *d_itoa_ll(long long nbr)
{
    int32 size;
    long long cpy = nbr;
    get_size_nb(cpy, size);
    char *buffer = malloc(sizeof(char) * (size + 1));
    if (buffer == NULL)
        return NULL;
    num_as_str(nbr, size, buffer);
    return buffer;
}

char *d_itoa_ll_no_alloc(long long nbr, char *buffer)
{
    int32 size;
    long long cpy = nbr;
    get_size_nb(cpy, size);
    num_as_str((usize)nbr, size, buffer);
    return buffer;
}

char *d_itoa_usize(usize nbr)
{
    int32 size;
    usize cpy = nbr;
    get_size_nb(cpy, size);
    char *buffer = malloc(sizeof(char) * (size + 1));
    if (buffer == NULL)
        return NULL;
    fill_buff_with_num_digits(buffer, nbr, size, 0);
    return buffer;
}

char *d_itoa_usize_no_alloc(usize nbr, char *buffer)
{
    int32 size;
    usize cpy = nbr;
    get_size_nb(cpy, size);
    fill_buff_with_num_digits(buffer, nbr, size, 0);
    return buffer;
}