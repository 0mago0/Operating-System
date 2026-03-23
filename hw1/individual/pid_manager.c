#include <stdio.h>
#include <stdlib.h>

#define MIN_PID 300
#define MAX_PID 5000
#define PID_COUNT (MAX_PID - MIN_PID + 1)
#define BITS_PER_BYTE 8
#define BITMAP_SIZE ((PID_COUNT + BITS_PER_BYTE - 1) / BITS_PER_BYTE)

static unsigned char *pid_bitmap = NULL;

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

    if (pid_bitmap == NULL) {
        return -1;
    }

    for (pid = MIN_PID; pid <= MAX_PID; pid++) {
        if (!is_pid_allocated(pid)) {
            mark_pid(pid);
            return pid;
        }
    }

    return -1;
}

void release_pid(int pid) {
    if (pid_bitmap == NULL) {
        return;
    }

    if (pid < MIN_PID || pid > MAX_PID) {
        fprintf(stderr, "Error: PID %d is out of range.\n", pid);
        return;
    }

    if (!is_pid_allocated(pid)) {
        fprintf(stderr, "Warning: PID %d was not allocated.\n", pid);
        return;
    }

    clear_pid(pid);
}

static void destroy_map(void) {
    free(pid_bitmap);
    pid_bitmap = NULL;
}

static int expect_int(const char *test_name, int expected, int actual) {
    if (expected != actual) {
        fprintf(stderr, "[FAIL] %s: expected %d, got %d\n",
                test_name, expected, actual);
        return 0;
    }

    printf("[PASS] %s\n", test_name);
    return 1;
}

static int test_basic_allocation_and_reuse(void) {
    int first_pid;
    int second_pid;
    int reused_pid;
    int passed = 1;

    first_pid = allocate_pid();
    second_pid = allocate_pid();
    release_pid(first_pid);
    reused_pid = allocate_pid();

    passed &= expect_int("first allocation returns MIN_PID", MIN_PID, first_pid);
    passed &= expect_int("second allocation returns next PID",
                         MIN_PID + 1, second_pid);
    passed &= expect_int("released PID is reused first", first_pid, reused_pid);

    return passed;
}

static int test_invalid_release_does_not_break_allocator(void) {
    int next_pid;
    int passed = 1;

    release_pid(MIN_PID - 1);
    release_pid(MAX_PID + 1);
    release_pid(MIN_PID + 100);

    next_pid = allocate_pid();
    passed &= expect_int("invalid releases do not change next allocation",
                         MIN_PID + 2, next_pid);

    return passed;
}

static int test_exhaustion_and_reuse_after_full(void) {
    int allocations_needed;
    int pid;
    int last_pid = -1;
    int exhausted_result;
    int reused_pid;
    int passed = 1;

    allocations_needed = PID_COUNT - 3;
    while (allocations_needed > 0) {
        pid = allocate_pid();
        if (pid == -1) {
            fprintf(stderr, "[FAIL] allocator ran out of PIDs too early\n");
            return 0;
        }
        last_pid = pid;
        allocations_needed--;
    }

    passed &= expect_int("last available PID is MAX_PID", MAX_PID, last_pid);

    exhausted_result = allocate_pid();
    passed &= expect_int("allocation fails when bitmap is full",
                         -1, exhausted_result);

    release_pid(MAX_PID);
    reused_pid = allocate_pid();
    passed &= expect_int("released PID can be reused after exhaustion",
                         MAX_PID, reused_pid);

    return passed;
}

int main(void) {
    int passed = 1;

    if (allocate_map() == -1) {
        fprintf(stderr, "Failed to create PID bitmap.\n");
        return 1;
    }

    passed &= test_basic_allocation_and_reuse();
    passed &= test_invalid_release_does_not_break_allocator();
    passed &= test_exhaustion_and_reuse_after_full();

    destroy_map();

    if (!passed) {
        fprintf(stderr, "PID manager tests failed.\n");
        return 1;
    }

    printf("All PID manager tests passed.\n");
    return 0;
}
