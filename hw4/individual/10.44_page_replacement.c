/**
 * Exercise 10.44 - FIFO, LRU, and Optimal Page Replacement
 *
 * Generates a random page-reference string with page numbers 0 through 9,
 * then counts the page faults produced by FIFO, LRU, and optimal replacement.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PAGE_MIN 0
#define PAGE_MAX 9
#define DEFAULT_REFERENCE_LENGTH 20

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <number_of_frames> [reference_length] [seed]\n", program_name);
    fprintf(stderr, "Example: %s 3\n", program_name);
    fprintf(stderr, "Example: %s 3 20 12345\n", program_name);
}

static int parse_positive_int(const char *text, const char *name) {
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0' || value <= 0) {
        fprintf(stderr, "%s must be a positive integer.\n", name);
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

static int find_page(const int frames[], int frame_count, int page) {
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == page) {
            return i;
        }
    }

    return -1;
}

static void generate_reference_string(int references[], int reference_length) {
    for (int i = 0; i < reference_length; i++) {
        references[i] = PAGE_MIN + rand() % (PAGE_MAX - PAGE_MIN + 1);
    }
}

static int count_fifo_faults(const int references[], int reference_length, int frame_count) {
    int *frames = malloc((size_t)frame_count * sizeof(int));
    int faults = 0;
    int next_replace = 0;
    int filled = 0;

    if (frames == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    for (int i = 0; i < reference_length; i++) {
        if (find_page(frames, frame_count, references[i]) != -1) {
            continue;
        }

        faults++;
        if (filled < frame_count) {
            frames[filled++] = references[i];
        } else {
            frames[next_replace] = references[i];
            next_replace = (next_replace + 1) % frame_count;
        }
    }

    free(frames);
    return faults;
}

static int count_lru_faults(const int references[], int reference_length, int frame_count) {
    int *frames = malloc((size_t)frame_count * sizeof(int));
    int *last_used = malloc((size_t)frame_count * sizeof(int));
    int faults = 0;
    int filled = 0;

    if (frames == NULL || last_used == NULL) {
        perror("malloc");
        free(frames);
        free(last_used);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    for (int time_index = 0; time_index < reference_length; time_index++) {
        int page = references[time_index];
        int frame_index = find_page(frames, frame_count, page);

        if (frame_index != -1) {
            last_used[frame_index] = time_index;
            continue;
        }

        faults++;
        if (filled < frame_count) {
            frames[filled] = page;
            last_used[filled] = time_index;
            filled++;
        } else {
            int lru_index = 0;
            for (int i = 1; i < frame_count; i++) {
                if (last_used[i] < last_used[lru_index]) {
                    lru_index = i;
                }
            }

            frames[lru_index] = page;
            last_used[lru_index] = time_index;
        }
    }

    free(frames);
    free(last_used);
    return faults;
}

static int find_next_use(const int references[], int reference_length, int start_index, int page) {
    for (int i = start_index; i < reference_length; i++) {
        if (references[i] == page) {
            return i;
        }
    }

    return reference_length + 1;
}

static int count_opt_faults(const int references[], int reference_length, int frame_count) {
    int *frames = malloc((size_t)frame_count * sizeof(int));
    int faults = 0;
    int filled = 0;

    if (frames == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    for (int i = 0; i < reference_length; i++) {
        int page = references[i];

        if (find_page(frames, frame_count, page) != -1) {
            continue;
        }

        faults++;
        if (filled < frame_count) {
            frames[filled++] = page;
        } else {
            int replace_index = 0;
            int farthest_next_use = -1;

            for (int frame_index = 0; frame_index < frame_count; frame_index++) {
                int next_use = find_next_use(references, reference_length, i + 1, frames[frame_index]);
                if (next_use > farthest_next_use) {
                    farthest_next_use = next_use;
                    replace_index = frame_index;
                }
            }

            frames[replace_index] = page;
        }
    }

    free(frames);
    return faults;
}

static void print_reference_string(const int references[], int reference_length) {
    printf("Page reference string (%d references):\n", reference_length);
    for (int i = 0; i < reference_length; i++) {
        printf("%d", references[i]);
        if (i < reference_length - 1) {
            printf(" ");
        }
    }
    printf("\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int frame_count = parse_positive_int(argv[1], "number_of_frames");
    int reference_length = DEFAULT_REFERENCE_LENGTH;
    unsigned int seed = (unsigned int)time(NULL);

    if (argc >= 3) {
        reference_length = parse_positive_int(argv[2], "reference_length");
    }

    if (argc == 4) {
        seed = parse_seed(argv[3]);
    }

    int *references = malloc((size_t)reference_length * sizeof(int));
    if (references == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    srand(seed);
    generate_reference_string(references, reference_length);

    printf("Frames: %d\n", frame_count);
    printf("Seed: %u\n\n", seed);
    print_reference_string(references, reference_length);

    printf("Page faults:\n");
    printf("FIFO: %d\n", count_fifo_faults(references, reference_length, frame_count));
    printf("LRU : %d\n", count_lru_faults(references, reference_length, frame_count));
    printf("OPT : %d\n", count_opt_faults(references, reference_length, frame_count));

    free(references);
    return EXIT_SUCCESS;
}
