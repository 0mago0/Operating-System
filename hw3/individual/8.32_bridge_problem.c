/**
 * Exercise 8.32 - The Bridge Problem using POSIX Synchronization
 * 
 * A single-lane bridge connects North Tunbridge and South Tunbridge.
 * Farmers from each village use the bridge, but a northbound and southbound
 * farmer cannot be on the bridge at the same time (deadlock prevention).
 * 
 * Uses semaphores and mutex locks to prevent deadlock.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Semaphores for synchronization
sem_t bridge;           // Binary semaphore for bridge access

// Mutex for protecting counters
pthread_mutex_t lock;

// Counters
int northbound_on_bridge = 0;
int southbound_on_bridge = 0;
int northbound_waiting = 0;
int southbound_waiting = 0;

// Statistics
int northbound_crossed = 0;
int southbound_crossed = 0;

/**
 * Small per-thread pseudo-random generator.
 */
unsigned int next_random(unsigned int* seed) {
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

/**
 * Thread function for northbound farmer
 */
void* northbound_farmer(void* arg) {
    int farmer_id = *(int*)arg;
    free(arg);
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(farmer_id * 2654435761u);
    
    // Want to cross the bridge
    pthread_mutex_lock(&lock);
    northbound_waiting++;
    printf("[North %d] Waiting to cross (waiting: %d)\n", farmer_id, northbound_waiting);
    pthread_mutex_unlock(&lock);
    
    // Only one farmer may use the single-lane bridge at a time.
    sem_wait(&bridge);
    
    // Enter bridge
    pthread_mutex_lock(&lock);
    northbound_waiting--;
    northbound_on_bridge++;
    printf("[North %d] Crossing bridge... (on bridge: %d)\n", farmer_id, northbound_on_bridge);
    pthread_mutex_unlock(&lock);
    
    // Simulate crossing
    sleep(next_random(&seed) % 3 + 1);
    
    // Exit bridge
    pthread_mutex_lock(&lock);
    northbound_on_bridge--;
    northbound_crossed++;
    printf("[North %d] Finished crossing (total crossed: %d)\n", farmer_id, northbound_crossed);
    pthread_mutex_unlock(&lock);
    
    sem_post(&bridge);
    
    pthread_exit(NULL);
}

/**
 * Thread function for southbound farmer
 */
void* southbound_farmer(void* arg) {
    int farmer_id = *(int*)arg;
    free(arg);
    unsigned int seed = ((unsigned int)time(NULL) + 1000u) ^ (unsigned int)(farmer_id * 2654435761u);
    
    // Want to cross the bridge
    pthread_mutex_lock(&lock);
    southbound_waiting++;
    printf("[South %d] Waiting to cross (waiting: %d)\n", farmer_id, southbound_waiting);
    pthread_mutex_unlock(&lock);
    
    // Only one farmer may use the single-lane bridge at a time.
    sem_wait(&bridge);
    
    // Enter bridge
    pthread_mutex_lock(&lock);
    southbound_waiting--;
    southbound_on_bridge++;
    printf("[South %d] Crossing bridge... (on bridge: %d)\n", farmer_id, southbound_on_bridge);
    pthread_mutex_unlock(&lock);
    
    // Simulate crossing
    sleep(next_random(&seed) % 3 + 1);
    
    // Exit bridge
    pthread_mutex_lock(&lock);
    southbound_on_bridge--;
    southbound_crossed++;
    printf("[South %d] Finished crossing (total crossed: %d)\n", farmer_id, southbound_crossed);
    pthread_mutex_unlock(&lock);
    
    sem_post(&bridge);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <northbound_farmers> <southbound_farmers>\n", argv[0]);
        return 1;
    }
    
    int num_north = atoi(argv[1]);
    int num_south = atoi(argv[2]);
    
    if (num_north <= 0 || num_south <= 0) {
        fprintf(stderr, "Invalid arguments: must be positive integers\n");
        return 1;
    }
    
    // Initialize semaphores and mutex
    sem_init(&bridge, 0, 1);
    pthread_mutex_init(&lock, NULL);
    
    int total_farmers = num_north + num_south;
    pthread_t* threads = (pthread_t*)malloc(total_farmers * sizeof(pthread_t));
    
    printf("Bridge Problem Simulation\n");
    printf("Northbound farmers: %d, Southbound farmers: %d\n\n", num_north, num_south);
    
    // Create northbound farmer threads
    for (int i = 0; i < num_north; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[i], NULL, northbound_farmer, id) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Create southbound farmer threads
    for (int i = 0; i < num_south; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[num_north + i], NULL, southbound_farmer, id) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < total_farmers; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print final statistics
    printf("\n=== Simulation Complete ===\n");
    printf("Northbound farmers crossed: %d\n", northbound_crossed);
    printf("Southbound farmers crossed: %d\n", southbound_crossed);
    
    // Cleanup
    sem_destroy(&bridge);
    pthread_mutex_destroy(&lock);
    free(threads);
    
    return 0;
}
