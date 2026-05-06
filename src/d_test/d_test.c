#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <signal.h>
#include "d_test.h"
#include "d_types.h"
#include "d_utils.h"

#define PRINT_SUCCESS_TEST(message) (printf(GREEN message RESET))

void d_test_impl(bool eval, const char *test, const char *file, int line)
{

    if (eval)
        PRINT_SUCCESS_TEST("OK! ");
    else
    {
        printf(RED "KO! expected (%s) at %s:%d" RESET, test, file, line);
        exit(EXIT_FAILURE);
    }
}

const char *signal_name(int sig)
{
    switch (sig)
    {
    case SIGINT:
        return "SIGINT";
    case SIGTERM:
        return "SIGTERM";
    case SIGKILL:
        return "SIGKILL";
    case SIGSEGV:
        return "SIGSEGV";
    case SIGQUIT:
        return "SIGQUIT";
    default:
        return "UNKNOWN";
    }
}

void d_test_run_tests_impl(DTest tests[], usize nb_test)
{
    usize failed = 0;
    for (size_t i = 0; i < nb_test; i++)
    {
        printf("%s: ", tests[i].test_name);
        fflush(NULL);
        pid_t pid = fork();
        if (pid == -1)
            return;
        if (pid == 0)
        {
            tests[i].test_fn();
            exit(0);
        }
        int status;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) != 0)
            failed++;
        if (WIFSIGNALED(status))
        {
            printf(RED "%s" RESET, signal_name(WTERMSIG(status)));
        }
        puts("");
    }

    exit(failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}