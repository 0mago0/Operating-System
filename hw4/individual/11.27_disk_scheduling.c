/**
 * Exercise 11.27 - FCFS, SCAN, and C-SCAN Disk Scheduling
 *
 * Generates 1,000 random cylinder requests for a disk with cylinders 0 to 4,999,
 * then reports total head movement for FCFS, SCAN, and C-SCAN scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CYLINDER_MIN 0
#define CYLINDER_MAX 4999
#define REQUEST_COUNT 1000

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <initial_head_position> [seed]\n", program_name);
    fprintf(stderr, "Example: %s 2150\n", program_name);
    fprintf(stderr, "Example: %s 2150 12345\n", program_name);
}

static int parse_head_position(const char *text) {
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0' || value < CYLINDER_MIN || value > CYLINDER_MAX) {
        fprintf(stderr, "initial_head_position must be between %d and %d.\n", CYLINDER_MIN, CYLINDER_MAX);
        exit(EXIT_FAILURE);
    }

    return (int)value;
}

static unsigned int parse_seed(const char *text) {
    char *end = NULL;
    unsigned long value = strtoul(text, &end, 10);

    if (*text == '\0' || *end != '\0') {
        fprintf(stderr, "seed must be a non-negative integer.\n");
        exit(EXIT_FAILURE);
    }

    return (unsigned int)value;
}

static int compare_ints(const void *left, const void *right) {
    int a = *(const int *)left;
    int b = *(const int *)right;

    return (a > b) - (a < b);
}

static int abs_difference(int a, int b) {
    return a > b ? a - b : b - a;
}

static void generate_requests(int requests[]) {
    for (int i = 0; i < REQUEST_COUNT; i++) {
        requests[i] = CYLINDER_MIN + rand() % (CYLINDER_MAX - CYLINDER_MIN + 1);
    }
}

static long total_fcfs_movement(const int requests[], int initial_head) {
    long movement = 0;
    int current = initial_head;

    for (int i = 0; i < REQUEST_COUNT; i++) {
        movement += abs_difference(current, requests[i]);
        current = requests[i];
    }

    return movement;
}

static int first_request_at_or_after_head(const int sorted_requests[], int initial_head) {
    int left = 0;
    int right = REQUEST_COUNT;

    while (left < right) {
        int middle = left + (right - left) / 2;
        if (sorted_requests[middle] < initial_head) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    return left;
}

static long total_scan_movement(const int sorted_requests[], int initial_head) {
    long movement = 0;
    int current = initial_head;
    int split = first_request_at_or_after_head(sorted_requests, initial_head);

    for (int i = split; i < REQUEST_COUNT; i++) {
        movement += abs_difference(current, sorted_requests[i]);
        current = sorted_requests[i];
    }

    if (split > 0) {
        movement += abs_difference(current, CYLINDER_MAX);
        current = CYLINDER_MAX;

        for (int i = split - 1; i >= 0; i--) {
            movement += abs_difference(current, sorted_requests[i]);
            current = sorted_requests[i];
        }
    }

    return movement;
}

static long total_cscan_movement(const int sorted_requests[], int initial_head) {
    long movement = 0;
    int current = initial_head;
    int split = first_request_at_or_after_head(sorted_requests, initial_head);

    for (int i = split; i < REQUEST_COUNT; i++) {
        movement += abs_difference(current, sorted_requests[i]);
        current = sorted_requests[i];
    }

    if (split > 0) {
        movement += abs_difference(current, CYLINDER_MAX);
        movement += CYLINDER_MAX - CYLINDER_MIN;
        current = CYLINDER_MIN;

        for (int i = 0; i < split; i++) {
            movement += abs_difference(current, sorted_requests[i]);
            current = sorted_requests[i];
        }
    }

    return movement;
}

static void print_request_sample(const int requests[]) {
    printf("First 20 generated requests:\n");
    for (int i = 0; i < 20; i++) {
        printf("%d", requests[i]);
        if (i < 19) {
            printf(" ");
        }
    }
    printf("\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int initial_head = parse_head_position(argv[1]);
    unsigned int seed = (unsigned int)time(NULL);

    if (argc == 3) {
        seed = parse_seed(argv[2]);
    }

    int requests[REQUEST_COUNT];
    int sorted_requests[REQUEST_COUNT];

    srand(seed);
    generate_requests(requests);

    for (int i = 0; i < REQUEST_COUNT; i++) {
        sorted_requests[i] = requests[i];
    }
    qsort(sorted_requests, REQUEST_COUNT, sizeof(int), compare_ints);

    printf("Initial head position: %d\n", initial_head);
    printf("Cylinder range: %d to %d\n", CYLINDER_MIN, CYLINDER_MAX);
    printf("Request count: %d\n", REQUEST_COUNT);
    printf("Seed: %u\n\n", seed);
    print_request_sample(requests);

    printf("Total head movement:\n");
    printf("FCFS  : %ld\n", total_fcfs_movement(requests, initial_head));
    printf("SCAN  : %ld\n", total_scan_movement(sorted_requests, initial_head));
    printf("C-SCAN: %ld\n", total_cscan_movement(sorted_requests, initial_head));

    return EXIT_SUCCESS;
}
