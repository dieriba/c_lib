#include <general_lib.h>
#include <math.h>


usize   get_number_len_int32(int32 nb)
{
    usize   len = 0;
    usize to_add = (nb < 0) + (nb == 0);

    while (nb != 0)
    {
        nb /= 10;
        ++len;
    }

    return len + to_add;
}

usize   get_number_len_usize(usize nb)
{
    usize len = (nb == 0);

    while (nb != 0)
    {
        nb /= 10;
        ++len;
    }

    return len;
}

char* d_itoa_i32(int32 nb)
{
    usize len = get_number_len_int32(nb);
    char* str = malloc(sizeof(char) * (len + 1));
    
    str[len--] = 0;
    usize stop = (nb < 0);
    str[stop] = '-';
    
    if (str == NULL)
        return NULL;
    for (size_t i = len; i >= stop; --i)
    {
        str[i] = (nb % 10) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nb /= 10;
    }
    return str;
}

char* d_itoa_i32_no_alloc(int32 nb)
{
    static char str[12];
    usize len = get_number_len_int32(nb);

    str[len--] = 0;
    usize stop = (nb < 0);
    str[stop] = '-';

    for (size_t i = len; i >= stop; --i)
    {
        str[i] = (nb % 10) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nb /= 10;
    }

    return str;
}

char *d_itoa_usize(usize nb)
{
    usize len = get_number_len_usize(nb);
    char* str = malloc(sizeof(char) * (len + 1));
    if (str == NULL)
        return NULL;
    usize factor = (usize)pow((double)10, (double)len);
    str[len--] = 0;
    for (size_t i = 0; i < len; ++i)
    {
        str[i] = (nb / factor) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        factor /= 10;
    }
    return str;
}

char* d_itoa_usize_no_alloc(usize nb)
{
    static char str[21];
    usize len = get_number_len_usize(nb);
    usize factor = (usize)pow((double)10, (double)len);
    str[len--] = 0;
    for (size_t i = 0; i < len; ++i)
    {
        str[i] = (nb / factor) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        factor /= 10;
    }
    return str;
}