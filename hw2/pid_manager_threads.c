#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MIN_PID 300
#define MAX_PID 5000
#define PID_COUNT (MAX_PID - MIN_PID + 1)
#define BITS_PER_BYTE 8
#define BITMAP_SIZE ((PID_COUNT + BITS_PER_BYTE - 1) / BITS_PER_BYTE)
#define DEFAULT_THREAD_COUNT 100
#define DEFAULT_MAX_SLEEP_SECONDS 3

static unsigned char *pid_bitmap = NULL;
static pthread_mutex_t pid_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int thread_number;
    int max_sleep_seconds;
    unsigned int seed;
} worker_data_t;

static int parse_positive_int(const char *text, int *value) {
    char *endptr;
    long parsed;

    errno = 0;
    parsed = strtol(text, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || parsed <= 0 || parsed > 1000000) {
        return -1;
    }

    *value = (int)parsed;
    return 0;
}

static int pid_to_index(int pid) {
    return pid - MIN_PID;
}

static int is_pid_allocated(int pid) {
    int index = pid_to_index(pid);
    int byte_index = index / BITS_PER_BYTE;
    int bit_index = index % BITS_PER_BYTE;

    return (pid_bitmap[byte_index] & (1U << bit_index)) != 0;
}

static void mark_pid(int pid) {
    int index = pid_to_index(pid);
    int byte_index = index / BITS_PER_BYTE;
    int bit_index = index % BITS_PER_BYTE;

    pid_bitmap[byte_index] |= (1U << bit_index);
}

static void clear_pid(int pid) {
    int index = pid_to_index(pid);
    int byte_index = index / BITS_PER_BYTE;
    int bit_index = index % BITS_PER_BYTE;

    pid_bitmap[byte_index] &= ~(1U << bit_index);
}

int allocate_map(void) {
    pid_bitmap = calloc(BITMAP_SIZE, sizeof(unsigned char));
    if (pid_bitmap == NULL) {
        return -1;
    }

    return 1;
}

int allocate_pid(void) {
    int pid;

    pthread_mutex_lock(&pid_lock);

    if (pid_bitmap == NULL) {
        pthread_mutex_unlock(&pid_lock);
        return -1;
    }

    for (pid = MIN_PID; pid <= MAX_PID; pid++) {
        if (!is_pid_allocated(pid)) {
            mark_pid(pid);
            pthread_mutex_unlock(&pid_lock);
            return pid;
        }
    }

    pthread_mutex_unlock(&pid_lock);
    return -1;
}

void release_pid(int pid) {
    pthread_mutex_lock(&pid_lock);

    if (pid_bitmap != NULL && pid >= MIN_PID && pid <= MAX_PID &&
        is_pid_allocated(pid)) {
        clear_pid(pid);
    }

    pthread_mutex_unlock(&pid_lock);
}

static void destroy_map(void) {
    free(pid_bitmap);
    pid_bitmap = NULL;
}

static void *pid_worker(void *arg) {
    worker_data_t *data = (worker_data_t *)arg;
    int pid;
    unsigned int sleep_time;

    pid = allocate_pid();
    if (pid == -1) {
        fprintf(stderr, "Thread %d could not allocate a PID.\n",
                data->thread_number);
        return NULL;
    }

    sleep_time = (unsigned int)(rand_r(&data->seed) %
                                data->max_sleep_seconds) + 1U;

    printf("Thread %d allocated PID %d for %u second(s).\n",
           data->thread_number, pid, sleep_time);
    sleep(sleep_time);
    release_pid(pid);
    printf("Thread %d released PID %d.\n", data->thread_number, pid);

    return NULL;
}

int main(int argc, char *argv[]) {
    int thread_count = DEFAULT_THREAD_COUNT;
    int max_sleep_seconds = DEFAULT_MAX_SLEEP_SECONDS;
    pthread_t *threads;
    worker_data_t *worker_data;
    int i;
    int result;

    if (argc > 3) {
        fprintf(stderr, "Usage: %s [thread_count] [max_sleep_seconds]\n",
                argv[0]);
        return 1;
    }

    if (argc >= 2 &&
        parse_positive_int(argv[1], &thread_count) == -1) {
        fprintf(stderr, "Error: thread_count must be a positive integer.\n");
        return 1;
    }

    if (argc == 3 &&
        parse_positive_int(argv[2], &max_sleep_seconds) == -1) {
        fprintf(stderr,
                "Error: max_sleep_seconds must be a positive integer.\n");
        return 1;
    }

    if (allocate_map() == -1) {
        perror("allocate_map failed");
        return 1;
    }

    threads = malloc((size_t)thread_count * sizeof(pthread_t));
    worker_data = calloc((size_t)thread_count, sizeof(worker_data_t));
    if (threads == NULL || worker_data == NULL) {
        perror("memory allocation failed");
        free(threads);
        free(worker_data);
        destroy_map();
        return 1;
    }

    for (i = 0; i < thread_count; i++) {
        worker_data[i].thread_number = i + 1;
        worker_data[i].max_sleep_seconds = max_sleep_seconds;
        worker_data[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 97);

        result = pthread_create(&threads[i], NULL, pid_worker,
                                &worker_data[i]);
        if (result != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(result));
            thread_count = i;
            break;
        }
    }

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(result));
        }
    }

    free(threads);
    free(worker_data);
    destroy_map();

    printf("PID manager multithreaded test finished.\n");
    return 0;
}
