#define _POSIX_C_SOURCE 200809L

#pragma STDC FENV_ACCESS ON

#include <fenv.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static sigjmp_buf resume_env;
static volatile sig_atomic_t fault_count = 0;
static volatile double denominator_state = 0.0;

static void handle_sigfpe(int sig) {
    (void)sig;
    ++fault_count;
    const char message[] = "Caught SIGFPE (floating point exception). Attempting to recover...\n";
    (void)write(STDERR_FILENO, message, sizeof(message) - 1);
    siglongjmp(resume_env, 1);
}

int main(void) {
    if (signal(SIGFPE, handle_sigfpe) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    volatile double numerator = 42.0;
    denominator_state = 0.0;

    if (sigsetjmp(resume_env, 1) != 0) {
        if (fault_count == 1) {
            denominator_state = 1.0;
            puts("Recovered from SIGFPE; continuing with a safe denominator.");
        } else {
            printf("Handled %d SIGFPE signals; exiting cleanly.\n", (int)fault_count);
            return EXIT_SUCCESS;
        }
    }

    feclearexcept(FE_ALL_EXCEPT);

    if (fault_count == 0) {
        puts("Attempting to divide by zero to trigger SIGFPE...");
    } else {
        puts("Attempting safe division after handling SIGFPE...");
    }
    fflush(stdout);

    volatile double result = numerator / denominator_state;

    if (fetestexcept(FE_DIVBYZERO)) {
        raise(SIGFPE);
    }

    printf("Computation completed successfully. Result = %.2f\n", result);
    return EXIT_SUCCESS;
}
