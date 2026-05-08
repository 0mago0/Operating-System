/**
 * Chapter 7 Project 3 - The Dining Philosophers Problem
 * 
 * Five philosophers sit at a round table with a single fork between each pair.
 * Each philosopher needs two forks to eat. This solution uses POSIX mutex locks
 * and condition variables to prevent deadlock.
 * 
 * Solution approach: Use a "waiter" (resource manager) approach to coordinate
 * fork access and prevent all philosophers from picking up the same fork.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5

// States for each philosopher
typedef enum {
    THINKING,
    HUNGRY,
    EATING
} state_t;

// Global state
state_t state[NUM_PHILOSOPHERS];
pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t state_cond[NUM_PHILOSOPHERS];

// Statistics
int eat_count[NUM_PHILOSOPHERS] = {0};

/**
 * Small per-thread pseudo-random generator.
 */
unsigned int next_random(unsigned int* seed) {
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

/**
 * Check if a philosopher can eat (has both adjacent forks available)
 */
bool can_eat(int philosopher) {
    int left = (philosopher + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (philosopher + 1) % NUM_PHILOSOPHERS;
    
    return (state[philosopher] == HUNGRY && 
            state[left] != EATING && 
            state[right] != EATING);
}

/**
 * Test and wake up neighbors if they can eat
 */
void test_and_signal(int philosopher) {
    int left = (philosopher + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (philosopher + 1) % NUM_PHILOSOPHERS;
    
    if (can_eat(left)) {
        state[left] = EATING;
        pthread_cond_signal(&state_cond[left]);
    }
    
    if (can_eat(right)) {
        state[right] = EATING;
        pthread_cond_signal(&state_cond[right]);
    }
}

/**
 * Philosopher picks up forks (acquires resources)
 */
void pickup_forks(int philosopher) {
    pthread_mutex_lock(&state_lock);
    
    state[philosopher] = HUNGRY;
    printf("[%d] Philosopher %d is hungry, waiting for forks...\n", (int)time(NULL), philosopher);
    
    // Test if we can eat immediately
    if (can_eat(philosopher)) {
        state[philosopher] = EATING;
        printf("[%d] Philosopher %d picked up forks, starting to eat\n", (int)time(NULL), philosopher);
    } else {
        // Wait until both adjacent philosophers are not eating
        while (state[philosopher] != EATING) {
            pthread_cond_wait(&state_cond[philosopher], &state_lock);
        }
        printf("[%d] Philosopher %d picked up forks, starting to eat\n", (int)time(NULL), philosopher);
    }
    
    pthread_mutex_unlock(&state_lock);
}

/**
 * Philosopher puts down forks (releases resources)
 */
void putdown_forks(int philosopher) {
    pthread_mutex_lock(&state_lock);
    
    state[philosopher] = THINKING;
    printf("[%d] Philosopher %d finished eating, thinking\n", (int)time(NULL), philosopher);
    eat_count[philosopher]++;
    
    // Test left and right neighbors
    test_and_signal(philosopher);
    
    pthread_mutex_unlock(&state_lock);
}

/**
 * Philosopher thread function
 */
void* philosopher(void* arg) {
    int philosopher_id = *(int*)arg;
    free(arg);
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(philosopher_id * 2654435761u);
    
    for (int i = 0; i < 3; i++) {
        // Think
        int think_time = (int)(next_random(&seed) % 3) + 1;
        printf("[%d] Philosopher %d thinking for %d seconds\n", (int)time(NULL), philosopher_id, think_time);
        sleep(think_time);
        
        // Eat
        pickup_forks(philosopher_id);
        int eat_time = (int)(next_random(&seed) % 2) + 1;
        sleep(eat_time);
        putdown_forks(philosopher_id);
    }
    
    printf("Philosopher %d finished\n", philosopher_id);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[NUM_PHILOSOPHERS];
    
    printf("Dining Philosophers Problem\n");
    printf("===========================\n\n");
    fflush(stdout);
    
    // Initialize condition variables
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_init(&state_cond[i], NULL);
        state[i] = THINKING;
    }
    
    // Create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[i], NULL, philosopher, id) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all philosophers to finish
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print statistics
    printf("\n===== Final Statistics =====\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times\n", i, eat_count[i]);
    }
    
    // Cleanup
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&state_cond[i]);
    }
    pthread_mutex_destroy(&state_lock);
    
    return 0;
}
