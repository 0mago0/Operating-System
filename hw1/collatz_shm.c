#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_NAME "/collatz_shm_example"
#define SHM_SIZE 65536

static int append_number(char *buffer, size_t buffer_size, size_t *offset,
                         long value, int add_separator) {
    int written;

    written = snprintf(buffer + *offset, buffer_size - *offset,
                       add_separator ? ", %ld" : "%ld", value);

    if (written < 0 || (size_t)written >= buffer_size - *offset) {
        return -1;
    }

    *offset += (size_t)written;
    return 0;
}

int main(int argc, char *argv[]) {
    char *endptr;
    long n;
    int shm_fd;
    char *shared_memory;
    pid_t pid;
    size_t offset = 0;
    long current;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <positive integer>\n", argv[0]);
        return 1;
    }

    errno = 0;
    n = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || n <= 0) {
        fprintf(stderr, "Error: argument must be a valid positive integer.\n");
        return 1;
    }

    shm_unlink(SHM_NAME);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    shared_memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                         shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    memset(shared_memory, 0, SHM_SIZE);

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        munmap(shared_memory, SHM_SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    if (pid == 0) {
        current = n;

        if (append_number(shared_memory, SHM_SIZE, &offset, current, 0) == -1) {
            fprintf(stderr, "Error: shared memory buffer is too small.\n");
            munmap(shared_memory, SHM_SIZE);
            close(shm_fd);
            return 1;
        }

        while (current != 1) {
            if (current % 2 == 0) {
                current /= 2;
            } else {
                current = 3 * current + 1;
            }

            if (append_number(shared_memory, SHM_SIZE, &offset, current, 1) == -1) {
                fprintf(stderr, "Error: shared memory buffer is too small.\n");
                munmap(shared_memory, SHM_SIZE);
                close(shm_fd);
                return 1;
            }
        }

        munmap(shared_memory, SHM_SIZE);
        close(shm_fd);
        return 0;
    }

    wait(NULL);
    printf("%s\n", shared_memory);

    munmap(shared_memory, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 0;
}
