#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handle_sigsegv(int sig) {
    (void)sig;
    const char message[] = "Segmentation fault caught! Cleaning up and exiting.\n";
    (void)write(STDERR_FILENO, message, sizeof(message) - 1);
    _exit(EXIT_FAILURE);
}

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
