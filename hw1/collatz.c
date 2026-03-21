#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    char *endptr;
    long n;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <positive integer>\n", argv[0]);
        return 1;
    }

    errno = 0;
    n = strtol(argv[1], &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: argument must be a valid positive integer.\n");
        return 1;
    }

    if (n <= 0) {
        fprintf(stderr, "Error: argument must be a positive integer greater than 0.\n");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
    else if (pid == 0) {
        printf("%ld", n);

        while (n != 1) {
            if (n % 2 == 0) {
                n = n / 2;
            } else {
                n = 3 * n + 1;
            }
            printf(", %ld", n);
        }
        printf("\n");
    }
    else {
        wait(NULL);
    }

    return 0;
}