/**
 * Exercise 7.14 - PID Manager with Thread Safety (Mutex Locks)
 * 
 * This extends a basic PID manager to be thread-safe.
 * Multiple threads can request and release PIDs concurrently without race conditions.
 * Uses mutex locks to protect the shared PID bitmap.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MIN_PID 300
#define MAX_PID 5000
#define PID_RANGE (MAX_PID - MIN_PID + 1)

// Global state
int pid_bitmap[PID_RANGE];  // 1 = allocated, 0 = available
pthread_mutex_t pid_lock;

/**
 * Small per-thread pseudo-random generator.
 */
unsigned int next_random(unsigned int* seed) {
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

/**
 * Initialize the PID manager
 */
void pid_init(void) {
    for (int i = 0; i < PID_RANGE; i++) {
        pid_bitmap[i] = 0;
    }
    pthread_mutex_init(&pid_lock, NULL);
}

/**
 * Allocate a PID
 * Returns: allocated PID, or -1 if none available
 */
int pid_allocate(void) {
    pthread_mutex_lock(&pid_lock);
    
    for (int i = 0; i < PID_RANGE; i++) {
        if (pid_bitmap[i] == 0) {
            pid_bitmap[i] = 1;
            pthread_mutex_unlock(&pid_lock);
            return MIN_PID + i;
        }
    }
    
    // No available PIDs
    pthread_mutex_unlock(&pid_lock);
    return -1;
}

/**
 * Release a PID
 * Returns: 1 if successful, 0 if PID not found
 */
int pid_release(int pid) {
    if (pid < MIN_PID || pid > MAX_PID) {
        return 0;
    }
    
    pthread_mutex_lock(&pid_lock);
    
    int index = pid - MIN_PID;
    if (pid_bitmap[index] == 0) {
        // PID not allocated
        pthread_mutex_unlock(&pid_lock);
        return 0;
    }
    
    pid_bitmap[index] = 0;
    pthread_mutex_unlock(&pid_lock);
    return 1;
}

/**
 * Report the number of available PIDs
 */
int pid_available_count(void) {
    pthread_mutex_lock(&pid_lock);
    
    int count = 0;
    for (int i = 0; i < PID_RANGE; i++) {
        if (pid_bitmap[i] == 0) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&pid_lock);
    return count;
}

/**
 * Thread function: requests and releases PIDs multiple times
 */
void* pid_worker(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(thread_id * 2654435761u);
    
    for (int i = 0; i < 5; i++) {
        // Request a PID
        int pid = pid_allocate();
        if (pid == -1) {
            printf("Thread %d: Failed to allocate PID (no available PIDs)\n", thread_id);
            break;
        }
        
        printf("Thread %d: Allocated PID %d\n", thread_id, pid);
        
        // Simulate work
        sleep(next_random(&seed) % 2 + 1);
        
        // Release the PID
        if (pid_release(pid)) {
            printf("Thread %d: Released PID %d\n", thread_id, pid);
        } else {
            printf("Thread %d: Failed to release PID %d\n", thread_id, pid);
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    if (num_threads <= 0 || num_threads > 100) {
        fprintf(stderr, "Invalid number of threads (1-100)\n");
        return 1;
    }
    
    // Initialize PID manager
    pid_init();
    
    printf("PID Manager initialized: range [%d, %d]\n", MIN_PID, MAX_PID);
    printf("Available PIDs: %d\n\n", pid_available_count());
    
    // Create threads
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    
    for (int i = 0; i < num_threads; i++) {
        int* tid = (int*)malloc(sizeof(int));
        *tid = i;
        
        if (pthread_create(&threads[i], NULL, pid_worker, tid) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Final Status ===\n");
    printf("Available PIDs: %d\n", pid_available_count());
    
    // Cleanup
    pthread_mutex_destroy(&pid_lock);
    free(threads);
    
    return 0;
}
