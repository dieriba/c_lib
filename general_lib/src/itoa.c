#include <general_lib.h>

int32   get_number_len_int32(int32 nb)
{
    int32   len = 0;
    int32   to_add = (nb <= 0);

    while (nb != 0)
    {
        nb /= 10;
        ++len;
    }

    return len + to_add;
}

int32   get_number_len_usize(usize nb)
{
    int32 len = (nb == 0);

    while (nb != 0)
    {
        nb /= 10;
        ++len;
    }

    return len;
}

char* d_itoa_i32(int32 nb)
{
    int64 nbr = nb; // storing int32 into int64 so further operation won't overflow
    int32 len = get_number_len_int32(nbr);
    int32 stop = (nbr < 0); // if nb < 0 stop = 1 else stop = 0
    nbr = (nbr > 0) * nbr + -(nbr < 0) * nbr; // that line just transform nbr into a positive number if it was negative
    char* str = malloc(sizeof(char) * (len + 1));
    if (str == NULL)
        return NULL;

    str[len--] = 0; //adding null bytes at end of str
    str[0] = '-'; // adding '-' char at index 0 by default it will be overwritten if stop = 0
    for (int32 i = len; i >= stop; --i)
    {
        str[i] = (nbr % 10) + 48; // nbr % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nbr /= 10;
    }
    return str;
}

char* d_itoa_i32_no_alloc(int32 nb, char* buffer)
{
    int64 nbr = nb; // storing int32 into int64 so further operation won't overflow
    int32 len = get_number_len_int32(nbr);
    int32 stop = (nbr < 0); // if nb < 0 stop = 1 else stop = 0
    nbr = (nbr > 0) * nbr + -(nbr < 0) * nbr; // that line just transform nbr into a positive number if it was negative
    
    buffer[len--] = 0; //adding null bytes at end of buffer
    buffer[0] = '-'; // adding '-' char at index 0 by default it will be overwritten if stop = 0
    for (int32 i = len; i >= stop; --i)
    {
        buffer[i] = (nbr % 10) + 48; // nbr % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nbr /= 10;
    }
    return buffer;
}

char *d_itoa_usize(usize nb)
{
    int32 len = get_number_len_usize(nb);
    char* str = malloc(sizeof(char) * (len + 1));
    if (str == NULL)
        return NULL;
    str[len--] = 0;
    for (int32 i = len; i >= 0; --i)
    {
        str[i] = (nb % 10) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nb /= 10;
    }
    return str;
}

char* d_itoa_usize_no_alloc(usize nb, char* buffer)
{
    int32 len = get_number_len_usize(nb);
    buffer[len--] = 0;
    for (int32 i = len; i >= 0; --i)
    {
        buffer[i] = (nb % 10) + 48; // nb % 10 will get the last unit of the numer and adding 48 will get us the ascii value of that number
        nb /= 10;
    }
    return buffer;
}