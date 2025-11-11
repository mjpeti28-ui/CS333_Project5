#define _POSIX_C_SOURCE 200809L

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static sigjmp_buf resume_env;
static volatile sig_atomic_t fault_count = 0;

static void handle_sigfpe(int sig) {
    (void)sig;
    ++fault_count;
    fputs("Caught SIGFPE (floating point exception). Recovering...\n", stderr);
    siglongjmp(resume_env, 1);
}

int main(void) {
    if (signal(SIGFPE, handle_sigfpe) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    int attempts = 0;

    if (sigsetjmp(resume_env, 1) != 0) {
        ++attempts;
        if (attempts > 1) {
            printf("Continuing execution after handling SIGFPE. Total faults: %d\n", (int)fault_count);
            return EXIT_SUCCESS;
        }
    }

    puts("Attempting to divide by zero to trigger SIGFPE...");
    volatile int numerator = 42;
    volatile int denominator = 0;
    volatile int result = numerator / denominator;
    (void)result;

    puts("This line will not execute on systems where SIGFPE triggers.");
    return EXIT_SUCCESS;
}
