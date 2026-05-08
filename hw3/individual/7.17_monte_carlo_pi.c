/**
 * Exercise 7.17 - Multithreaded Monte Carlo π Estimation
 * 
 * This program estimates the value of π using the Monte Carlo technique
 * with multiple threads. Each thread generates random points and determines
 * if they fall within a circle. A mutex lock protects the shared counter.
 */

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

// Global variables
int points_in_circle = 0;
int total_points = 0;
pthread_mutex_t lock;

// Structure to pass thread arguments
typedef struct {
    int thread_id;
    int points_per_thread;
} thread_args_t;

/**
 * Small per-thread pseudo-random generator.
 */
unsigned int next_random(unsigned int* seed) {
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

/**
 * Thread function: generates random points and checks if they fall within circle
 * Each thread generates its own random points independently
 */
void* generate_points(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    int local_count = 0;
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(args->thread_id * 2654435761u);
    
    for (int i = 0; i < args->points_per_thread; i++) {
        // Generate random point in [0,1] x [0,1]
        double x = (double)next_random(&seed) / UINT_MAX;
        double y = (double)next_random(&seed) / UINT_MAX;
        
        // Check if point is within the unit circle
        double distance = sqrt(x * x + y * y);
        if (distance <= 1.0) {
            local_count++;
        }
    }
    
    // Protect shared variable with mutex
    pthread_mutex_lock(&lock);
    points_in_circle += local_count;
    total_points += args->points_per_thread;
    pthread_mutex_unlock(&lock);
    
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <points_per_thread>\n", argv[0]);
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    int points_per_thread = atoi(argv[2]);
    
    if (num_threads <= 0 || points_per_thread <= 0) {
        fprintf(stderr, "Invalid arguments: must be positive integers\n");
        return 1;
    }
    
    // Initialize mutex
    pthread_mutex_init(&lock, NULL);
    
    // Create threads
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    
    for (int i = 0; i < num_threads; i++) {
        thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t));
        args->thread_id = i;
        args->points_per_thread = points_per_thread;
        
        if (pthread_create(&threads[i], NULL, generate_points, args) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Calculate π estimate
    // π/4 ≈ points_in_circle / total_points
    // π ≈ 4 * points_in_circle / total_points
    double pi_estimate = 4.0 * points_in_circle / total_points;
    double actual_pi = acos(-1.0);  // π = arccos(-1)
    
    printf("Total points: %d\n", total_points);
    printf("Points in circle: %d\n", points_in_circle);
    printf("Estimated π: %.6f\n", pi_estimate);
    printf("Actual π: %.6f\n", actual_pi);
    printf("Error: %.6f\n", fabs(pi_estimate - actual_pi));
    
    // Cleanup
    pthread_mutex_destroy(&lock);
    free(threads);
    
    return 0;
}
