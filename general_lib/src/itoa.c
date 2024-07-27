#include <general_lib.h>

char* d_itoa_i32(int32 nb)
{

}

char* d_itoa_i32_no_alloc(int32 nb)
{
    static char res[12];

    return res;
}

char *d_itoa_usize(usize nb)
{

}

char* d_itoa_usize_no_alloc(usize nb)
{
    static char res[21];

    return res;
}