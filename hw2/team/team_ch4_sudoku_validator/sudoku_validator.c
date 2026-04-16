#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 9
#define SUBGRID_SIZE 3
#define THREAD_COUNT 27

typedef enum {
    CHECK_ROW,
    CHECK_COLUMN,
    CHECK_SUBGRID
} check_type_t;

typedef struct {
    check_type_t type;
    int index;
    int start_row;
    int start_col;
    int (*grid)[SIZE];
    int is_valid;
} check_args_t;

static const int default_grid[SIZE][SIZE] = {
    {6, 2, 4, 5, 3, 9, 1, 8, 7},
    {5, 1, 9, 7, 2, 8, 6, 3, 4},
    {8, 3, 7, 6, 1, 4, 2, 9, 5},
    {1, 4, 3, 8, 6, 5, 7, 2, 9},
    {9, 5, 8, 2, 4, 7, 3, 6, 1},
    {7, 6, 2, 3, 9, 1, 4, 5, 8},
    {3, 7, 1, 9, 5, 6, 8, 4, 2},
    {4, 9, 6, 1, 8, 2, 5, 7, 3},
    {2, 8, 5, 4, 7, 3, 9, 1, 6}
};

static int parse_cell(const char *text, int *value) {
    char *endptr;
    long parsed;

    errno = 0;
    parsed = strtol(text, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || parsed < 1 || parsed > 9) {
        return -1;
    }

    *value = (int)parsed;
    return 0;
}

static int check_values(const int values[SIZE]) {
    int seen[SIZE + 1] = {0};
    int i;

    for (i = 0; i < SIZE; i++) {
        int value = values[i];

        if (value < 1 || value > 9 || seen[value]) {
            return 0;
        }

        seen[value] = 1;
    }

    return 1;
}

static void *validate_section(void *arg) {
    check_args_t *data = (check_args_t *)arg;
    int values[SIZE];
    int i;
    int row;
    int col;

    if (data->type == CHECK_ROW) {
        for (i = 0; i < SIZE; i++) {
            values[i] = data->grid[data->index][i];
        }
    } else if (data->type == CHECK_COLUMN) {
        for (i = 0; i < SIZE; i++) {
            values[i] = data->grid[i][data->index];
        }
    } else {
        i = 0;
        for (row = data->start_row; row < data->start_row + SUBGRID_SIZE;
             row++) {
            for (col = data->start_col; col < data->start_col + SUBGRID_SIZE;
                 col++) {
                values[i++] = data->grid[row][col];
            }
        }
    }

    data->is_valid = check_values(values);
    return NULL;
}

static void print_grid(int grid[SIZE][SIZE]) {
    int row;
    int col;

    printf("Sudoku grid:\n");
    for (row = 0; row < SIZE; row++) {
        for (col = 0; col < SIZE; col++) {
            printf("%d%s", grid[row][col], col == SIZE - 1 ? "" : " ");
        }
        printf("\n");
    }
    printf("\n");
}

static const char *check_label(check_type_t type) {
    if (type == CHECK_ROW) {
        return "Row";
    }
    if (type == CHECK_COLUMN) {
        return "Column";
    }
    return "Subgrid";
}

int main(int argc, char *argv[]) {
    int grid[SIZE][SIZE];
    pthread_t threads[THREAD_COUNT];
    check_args_t args[THREAD_COUNT];
    int thread_index = 0;
    int row;
    int col;
    int i;
    int result;
    int valid = 1;

    if (argc != 1 && argc != SIZE * SIZE + 1) {
        fprintf(stderr, "Usage: %s [81 values from 1 to 9]\n", argv[0]);
        return 1;
    }

    if (argc == 1) {
        memcpy(grid, default_grid, sizeof(default_grid));
    } else {
        for (i = 0; i < SIZE * SIZE; i++) {
            if (parse_cell(argv[i + 1], &grid[i / SIZE][i % SIZE]) == -1) {
                fprintf(stderr,
                        "Error: each Sudoku cell must be an integer from 1 to 9.\n");
                return 1;
            }
        }
    }

    print_grid(grid);

    for (row = 0; row < SIZE; row++) {
        args[thread_index].type = CHECK_ROW;
        args[thread_index].index = row;
        args[thread_index].start_row = row;
        args[thread_index].start_col = 0;
        args[thread_index].grid = grid;
        args[thread_index].is_valid = 0;
        thread_index++;
    }

    for (col = 0; col < SIZE; col++) {
        args[thread_index].type = CHECK_COLUMN;
        args[thread_index].index = col;
        args[thread_index].start_row = 0;
        args[thread_index].start_col = col;
        args[thread_index].grid = grid;
        args[thread_index].is_valid = 0;
        thread_index++;
    }

    for (row = 0; row < SIZE; row += SUBGRID_SIZE) {
        for (col = 0; col < SIZE; col += SUBGRID_SIZE) {
            args[thread_index].type = CHECK_SUBGRID;
            args[thread_index].index = thread_index - 18;
            args[thread_index].start_row = row;
            args[thread_index].start_col = col;
            args[thread_index].grid = grid;
            args[thread_index].is_valid = 0;
            thread_index++;
        }
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        result = pthread_create(&threads[i], NULL, validate_section, &args[i]);
        if (result != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(result));
            return 1;
        }
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "pthread_join failed: %s\n", strerror(result));
            return 1;
        }
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        if (!args[i].is_valid) {
            valid = 0;
            printf("%s %d is invalid.\n", check_label(args[i].type),
                   args[i].index + 1);
        }
    }

    if (valid) {
        printf("The Sudoku solution is valid.\n");
    } else {
        printf("The Sudoku solution is invalid.\n");
    }

    return valid ? 0 : 1;
}
