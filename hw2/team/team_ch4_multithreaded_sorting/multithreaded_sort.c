#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *array;
    int left;
    int right;
} sort_args_t;

typedef struct {
    int *array;
    int *result;
    int left;
    int middle;
    int right;
} merge_args_t;

static int compare_ints(const void *left, const void *right) {
    int a = *(const int *)left;
    int b = *(const int *)right;

    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

static int parse_int(const char *text, int *value) {
    char *endptr;
    long parsed;

    errno = 0;
    parsed = strtol(text, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || parsed < -1000000 ||
        parsed > 1000000) {
        return -1;
    }

    *value = (int)parsed;
    return 0;
}

static void *sort_sublist(void *arg) {
    sort_args_t *data = (sort_args_t *)arg;
    int length = data->right - data->left + 1;

    qsort(data->array + data->left, (size_t)length, sizeof(int),
          compare_ints);
    return NULL;
}

static void *merge_sublists(void *arg) {
    merge_args_t *data = (merge_args_t *)arg;
    int left_index = data->left;
    int right_index = data->middle + 1;
    int result_index = data->left;

    while (left_index <= data->middle && right_index <= data->right) {
        if (data->array[left_index] <= data->array[right_index]) {
            data->result[result_index++] = data->array[left_index++];
        } else {
            data->result[result_index++] = data->array[right_index++];
        }
    }

    while (left_index <= data->middle) {
        data->result[result_index++] = data->array[left_index++];
    }

    while (right_index <= data->right) {
        data->result[result_index++] = data->array[right_index++];
    }

    return NULL;
}

static void print_array(const char *label, const int *array, int length) {
    int i;

    printf("%s", label);
    for (i = 0; i < length; i++) {
        printf("%s%d", i == 0 ? "" : " ", array[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int default_values[] = {7, 12, 19, 3, 18, 4, 2, 6, 15, 8};
    int *numbers;
    int *sorted_numbers;
    int count;
    int middle;
    int i;
    pthread_t sort_threads[2];
    pthread_t merge_thread;
    sort_args_t sort_args[2];
    merge_args_t merge_args;
    int result;

    if (argc == 1) {
        count = (int)(sizeof(default_values) / sizeof(default_values[0]));
        numbers = malloc(sizeof(default_values));
        if (numbers == NULL) {
            perror("memory allocation failed");
            return 1;
        }
        memcpy(numbers, default_values, sizeof(default_values));
    } else {
        count = argc - 1;
        numbers = malloc((size_t)count * sizeof(int));
        if (numbers == NULL) {
            perror("memory allocation failed");
            return 1;
        }

        for (i = 0; i < count; i++) {
            if (parse_int(argv[i + 1], &numbers[i]) == -1) {
                fprintf(stderr, "Error: all inputs must be integers.\n");
                free(numbers);
                return 1;
            }
        }
    }

    if (count < 2) {
        fprintf(stderr, "Error: provide at least two integers to sort.\n");
        free(numbers);
        return 1;
    }

    sorted_numbers = malloc((size_t)count * sizeof(int));
    if (sorted_numbers == NULL) {
        perror("memory allocation failed");
        free(numbers);
        return 1;
    }

    middle = (count - 1) / 2;

    print_array("Original list: ", numbers, count);

    sort_args[0].array = numbers;
    sort_args[0].left = 0;
    sort_args[0].right = middle;
    sort_args[1].array = numbers;
    sort_args[1].left = middle + 1;
    sort_args[1].right = count - 1;

    for (i = 0; i < 2; i++) {
        result = pthread_create(&sort_threads[i], NULL, sort_sublist,
                                &sort_args[i]);
        if (result != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(result));
            free(numbers);
            free(sorted_numbers);
            return 1;
        }
    }

    for (i = 0; i < 2; i++) {
        result = pthread_join(sort_threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(result));
            free(numbers);
            free(sorted_numbers);
            return 1;
        }
    }

    print_array("Sorted left half: ", numbers, middle + 1);
    print_array("Sorted right half: ", numbers + middle + 1,
                count - middle - 1);

    merge_args.array = numbers;
    merge_args.result = sorted_numbers;
    merge_args.left = 0;
    merge_args.middle = middle;
    merge_args.right = count - 1;

    result = pthread_create(&merge_thread, NULL, merge_sublists, &merge_args);
    if (result != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(result));
        free(numbers);
        free(sorted_numbers);
        return 1;
    }

    result = pthread_join(merge_thread, NULL);
    if (result != 0) {
        fprintf(stderr, "pthread_join failed: %s\n", strerror(result));
        free(numbers);
        free(sorted_numbers);
        return 1;
    }

    print_array("Final sorted list: ", sorted_numbers, count);

    free(numbers);
    free(sorted_numbers);
    return 0;
}
