#include "d_error.h"

const char *d_error_message(DError err)
{
    if (err.msg)
        return err.msg;
    return d_error_print_result_as_str(err.code);
}
