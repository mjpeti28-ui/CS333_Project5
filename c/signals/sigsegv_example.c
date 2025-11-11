/**
 * @file sigsegv_example.c
 * @author Max Petite
 * @date 2025-11-11
 *
 * Shows how to intercept SIGSEGV and exit after notifying the user.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* SIGSEGV handler that notifies the user before exiting. */
static void handle_sigsegv(int sig) {
    (void)sig;
    const char message[] = "Segmentation fault caught! Cleaning up and exiting.\n";
    (void)write(STDERR_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_FAILURE);
}

/* Intentionally dereference NULL to demonstrate the handler. */
int main(void) {
    if (signal(SIGSEGV, handle_sigsegv) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    puts("Deliberately causing a segmentation fault...");
    fflush(stdout);

    int *ptr = NULL;
    *ptr = 1234;

    puts("This message will never be displayed.");
    return EXIT_SUCCESS;
}
