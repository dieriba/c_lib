#include <dstring.h>
#include <dutils.h>
#include <string.h>

void    print_test_status(char *test_description, char *expected, char *got, bool succeded)
{
    if (succeded == true)
    {
        printf(GREEN "Test[%s] succeded\n" RESET, test_description);
        return;
    }
    fprintf(stderr, RED "Test[%s] failed, expected[%s], got[%s]\n" RESET , test_description, expected, got);
}

void    test_d_string_new_with_string(void)
{
    DString* dstring = d_string_new_with_string("hello");
    char *str = "hello";
    print_test_status("string intern value should be `hello`", str, dstring -> string, strcmp(str, dstring -> string));
    
}

int main()
{

}