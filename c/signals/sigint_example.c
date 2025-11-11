#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handle_sigint(int sig) {
    (void)sig;
    const char message[] = "\nInterrupted! Exiting.\n";
    /* Use write in the handler to remain async-signal-safe. */
    (void)write(STDOUT_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_SUCCESS);
}

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
