#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    long long points_to_generate;
    unsigned int seed;
} thread_data_t;

static long long total_inside_circle = 0;
static pthread_mutex_t total_lock = PTHREAD_MUTEX_INITIALIZER;

static int parse_positive_long_long(const char *text, long long *value) {
    char *endptr;

    errno = 0;
    *value = strtoll(text, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || *value <= 0) {
        return -1;
    }

    return 0;
}

static double random_coordinate(unsigned int *seed) {
    return (2.0 * rand_r(seed) / (double)RAND_MAX) - 1.0;
}

static void *generate_points(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    long long inside_count = 0;
    long long i;

    for (i = 0; i < data->points_to_generate; i++) {
        double x = random_coordinate(&data->seed);
        double y = random_coordinate(&data->seed);

        if ((x * x) + (y * y) <= 1.0) {
            inside_count++;
        }
    }

    pthread_mutex_lock(&total_lock);
    total_inside_circle += inside_count;
    pthread_mutex_unlock(&total_lock);

    return NULL;
}

int main(int argc, char *argv[]) {
    long long total_points;
    long long thread_count;
    long long base_points;
    long long remaining_points;
    pthread_t *threads;
    thread_data_t *thread_data;
    unsigned int base_seed;
    long long i;
    long long created_threads = 0;
    double pi_estimate;
    int result;

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s <number_of_points> <number_of_threads> [seed]\n",
                argv[0]);
        return 1;
    }

    if (parse_positive_long_long(argv[1], &total_points) == -1) {
        fprintf(stderr, "Error: number_of_points must be a positive integer.\n");
        return 1;
    }

    if (parse_positive_long_long(argv[2], &thread_count) == -1) {
        fprintf(stderr, "Error: number_of_threads must be a positive integer.\n");
        return 1;
    }

    if (argc == 4) {
        long long parsed_seed;

        if (parse_positive_long_long(argv[3], &parsed_seed) == -1) {
            fprintf(stderr, "Error: seed must be a positive integer.\n");
            return 1;
        }

        base_seed = (unsigned int)parsed_seed;
    } else {
        base_seed = (unsigned int)time(NULL);
    }

    threads = malloc((size_t)thread_count * sizeof(pthread_t));
    thread_data = calloc((size_t)thread_count, sizeof(thread_data_t));
    if (threads == NULL || thread_data == NULL) {
        perror("memory allocation failed");
        free(threads);
        free(thread_data);
        return 1;
    }

    base_points = total_points / thread_count;
    remaining_points = total_points % thread_count;

    for (i = 0; i < thread_count; i++) {
        thread_data[i].points_to_generate = base_points;
        if (i < remaining_points) {
            thread_data[i].points_to_generate++;
        }

        thread_data[i].seed = base_seed + (unsigned int)(i * 97 + 13);

        result = pthread_create(&threads[i], NULL, generate_points,
                                &thread_data[i]);
        if (result != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(result));
            while (created_threads > 0) {
                created_threads--;
                pthread_join(threads[created_threads], NULL);
            }
            free(threads);
            free(thread_data);
            return 1;
        }

        created_threads++;
    }

    for (i = 0; i < thread_count; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(result));
            free(threads);
            free(thread_data);
            return 1;
        }
    }

    pi_estimate = 4.0 * total_inside_circle / total_points;

    printf("Total points: %lld\n", total_points);
    printf("Threads: %lld\n", thread_count);
    printf("Points inside circle: %lld\n", total_inside_circle);
    printf("Estimated pi: %.8f\n", pi_estimate);

    free(threads);
    free(thread_data);
    return 0;
}
