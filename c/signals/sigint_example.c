/**
 * @file sigint_example.c
 * @author Max Petite
 * @date 2025-11-11
 *
 * Registers a SIGINT handler that exits cleanly after printing a message.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Minimal SIGINT handler that writes a message then exits. */
static void handle_sigint(int sig) {
    (void)sig;
    const char message[] = "\nInterrupted! Exiting.\n";
    /* Use write in the handler to remain async-signal-safe. */
    (void)write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

/* Register the handler and wait for Ctrl+C forever. */
int main(void) {
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    puts("Press Ctrl+C to trigger SIGINT (Ctrl+C)...");
    fflush(stdout);

    while (1) {
        pause();
    }

    return EXIT_SUCCESS;
}
