#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_RESOURCES 5
#define DEFAULT_THREAD_COUNT 8
#define DEFAULT_MAX_REQUEST 3

static int available_resources = MAX_RESOURCES;
static pthread_mutex_t resource_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t resources_available = PTHREAD_COND_INITIALIZER;

typedef struct {
    int thread_number;
    int max_request;
    unsigned int seed;
} client_data_t;

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

int decrease_count(int count) {
    pthread_mutex_lock(&resource_lock);

    while (available_resources < count) {
        pthread_cond_wait(&resources_available, &resource_lock);
    }

    available_resources -= count;
    pthread_mutex_unlock(&resource_lock);
    return 0;
}

int increase_count(int count) {
    pthread_mutex_lock(&resource_lock);
    available_resources += count;
    pthread_cond_broadcast(&resources_available);
    pthread_mutex_unlock(&resource_lock);
    return 0;
}

static void *resource_client(void *arg) {
    client_data_t *data = (client_data_t *)arg;
    int requested;
    unsigned int hold_time;

    requested = (int)(rand_r(&data->seed) % data->max_request) + 1;
    hold_time = (unsigned int)(rand_r(&data->seed) % 3U) + 1U;

    printf("Client %d requesting %d resource(s).\n",
           data->thread_number, requested);
    decrease_count(requested);

    printf("Client %d acquired %d resource(s).\n",
           data->thread_number, requested);
    sleep(hold_time);

    increase_count(requested);
    printf("Client %d released %d resource(s).\n",
           data->thread_number, requested);

    return NULL;
}

int main(int argc, char *argv[]) {
    int thread_count = DEFAULT_THREAD_COUNT;
    int max_request = DEFAULT_MAX_REQUEST;
    pthread_t *threads;
    client_data_t *client_data;
    int i;
    int result;

    if (argc > 3) {
        fprintf(stderr, "Usage: %s [thread_count] [max_request]\n", argv[0]);
        return 1;
    }

    if (argc >= 2 &&
        parse_positive_int(argv[1], &thread_count) == -1) {
        fprintf(stderr, "Error: thread_count must be a positive integer.\n");
        return 1;
    }

    if (argc == 3 &&
        parse_positive_int(argv[2], &max_request) == -1) {
        fprintf(stderr, "Error: max_request must be a positive integer.\n");
        return 1;
    }

    if (max_request > MAX_RESOURCES) {
        fprintf(stderr, "Error: max_request cannot exceed MAX_RESOURCES (%d).\n",
                MAX_RESOURCES);
        return 1;
    }

    threads = malloc((size_t)thread_count * sizeof(pthread_t));
    client_data = calloc((size_t)thread_count, sizeof(client_data_t));
    if (threads == NULL || client_data == NULL) {
        perror("memory allocation failed");
        free(threads);
        free(client_data);
        return 1;
    }

    for (i = 0; i < thread_count; i++) {
        client_data[i].thread_number = i + 1;
        client_data[i].max_request = max_request;
        client_data[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 131);

        result = pthread_create(&threads[i], NULL, resource_client,
                                &client_data[i]);
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

    printf("Final available resources: %d\n", available_resources);

    free(threads);
    free(client_data);
    return 0;
}
