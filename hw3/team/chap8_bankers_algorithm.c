/**
 * Chapter 8 Project - Banker's Algorithm
 * 
 * Implements the Banker's Algorithm for deadlock avoidance.
 * Multiple customer threads request and release resources.
 * The banker checks if granting a request leads to a safe state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Number of resources types
#define NUM_RESOURCES 3
// Number of customers
#define NUM_CUSTOMERS 5

// Global data structures
int available[NUM_RESOURCES];
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];
int allocated[NUM_CUSTOMERS][NUM_RESOURCES];
int need[NUM_CUSTOMERS][NUM_RESOURCES];

pthread_mutex_t banker_lock = PTHREAD_MUTEX_INITIALIZER;

// Statistics
int request_count[NUM_CUSTOMERS] = {0};
int grant_count[NUM_CUSTOMERS] = {0};

/**
 * Small per-thread pseudo-random generator.
 */
unsigned int next_random(unsigned int* seed) {
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

/**
 * Check if a state is safe using the safety algorithm
 * Returns: true if safe, false if unsafe
 */
bool is_safe(void) {
    int work[NUM_RESOURCES];
    bool finish[NUM_CUSTOMERS] = {false};
    
    // Initialize work = available
    memcpy(work, available, sizeof(available));
    
    // Try to find a safe sequence
    for (int count = 0; count < NUM_CUSTOMERS; count++) {
        bool found = false;
        
        for (int customer = 0; customer < NUM_CUSTOMERS; customer++) {
            if (!finish[customer]) {
                // Check if customer can complete
                bool can_complete = true;
                for (int i = 0; i < NUM_RESOURCES; i++) {
                    if (need[customer][i] > work[i]) {
                        can_complete = false;
                        break;
                    }
                }
                
                if (can_complete) {
                    // Customer can complete, release resources
                    for (int i = 0; i < NUM_RESOURCES; i++) {
                        work[i] += allocated[customer][i];
                    }
                    finish[customer] = true;
                    found = true;
                    break;
                }
            }
        }
        
        if (!found) {
            // No safe sequence found
            return false;
        }
    }
    
    return true;
}

/**
 * Request resources (banker grants if safe)
 */
bool request_resources(int customer, int request[NUM_RESOURCES]) {
    pthread_mutex_lock(&banker_lock);
    
    // Check if request exceeds declared need or available resources
    for (int i = 0; i < NUM_RESOURCES; i++) {
        if (request[i] > need[customer][i]) {
            printf("Customer %d: Request denied (exceeds maximum need)\n", customer);
            pthread_mutex_unlock(&banker_lock);
            return false;
        }
        
        if (request[i] > available[i]) {
            printf("Customer %d: Request denied (not enough available)\n", customer);
            pthread_mutex_unlock(&banker_lock);
            return false;
        }
    }
    
    // Pretend to allocate and check if safe
    for (int i = 0; i < NUM_RESOURCES; i++) {
        available[i] -= request[i];
        allocated[customer][i] += request[i];
        need[customer][i] -= request[i];
    }
    
    if (is_safe()) {
        printf("Customer %d: Request GRANTED (resources: ", customer);
        for (int i = 0; i < NUM_RESOURCES; i++) {
            printf("%d ", request[i]);
        }
        printf(")\n");
        grant_count[customer]++;
        pthread_mutex_unlock(&banker_lock);
        return true;
    } else {
        // Rollback allocation
        for (int i = 0; i < NUM_RESOURCES; i++) {
            available[i] += request[i];
            allocated[customer][i] -= request[i];
            need[customer][i] += request[i];
        }
        printf("Customer %d: Request DENIED (would lead to unsafe state)\n", customer);
        pthread_mutex_unlock(&banker_lock);
        return false;
    }
}

/**
 * Release resources
 */
void release_resources(int customer, int release[NUM_RESOURCES]) {
    pthread_mutex_lock(&banker_lock);
    
    for (int i = 0; i < NUM_RESOURCES; i++) {
        if (release[i] > allocated[customer][i]) {
            printf("Customer %d: ERROR - releasing more than allocated\n", customer);
            pthread_mutex_unlock(&banker_lock);
            return;
        }
        
        available[i] += release[i];
        allocated[customer][i] -= release[i];
        need[customer][i] += release[i];
    }
    
    printf("Customer %d: Released resources (", customer);
    for (int i = 0; i < NUM_RESOURCES; i++) {
        printf("%d ", release[i]);
    }
    printf(")\n");
    
    pthread_mutex_unlock(&banker_lock);
}

/**
 * Generate a random request within the customer's remaining need.
 */
int random_request(int customer, unsigned int* seed, int request[NUM_RESOURCES]) {
    int total_request = 0;
    
    pthread_mutex_lock(&banker_lock);
    for (int j = 0; j < NUM_RESOURCES; j++) {
        request[j] = (int)(next_random(seed) % (unsigned int)(need[customer][j] + 1));
        total_request += request[j];
    }
    pthread_mutex_unlock(&banker_lock);
    
    return total_request;
}

/**
 * Generate a random release within the customer's current allocation.
 */
void random_release(int customer, unsigned int* seed, int release[NUM_RESOURCES]) {
    pthread_mutex_lock(&banker_lock);
    for (int j = 0; j < NUM_RESOURCES; j++) {
        release[j] = (int)(next_random(seed) % (unsigned int)(allocated[customer][j] + 1));
    }
    pthread_mutex_unlock(&banker_lock);
}

/**
 * Snapshot all resources allocated to a customer.
 */
void all_allocated_resources(int customer, int release[NUM_RESOURCES]) {
    pthread_mutex_lock(&banker_lock);
    memcpy(release, allocated[customer], sizeof(int) * NUM_RESOURCES);
    pthread_mutex_unlock(&banker_lock);
}

/**
 * Customer thread function
 */
void* customer(void* arg) {
    int customer_id = *(int*)arg;
    free(arg);
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(customer_id * 2654435761u);
    
    for (int i = 0; i < 3; i++) {
        // Request random resources
        int request[NUM_RESOURCES];
        int total_request = random_request(customer_id, &seed, request);
        
        if (total_request > 0) {
            request_count[customer_id]++;
            request_resources(customer_id, request);
        }
        
        // Simulate working
        sleep(next_random(&seed) % 2 + 1);
        
        // Release some resources
        int release[NUM_RESOURCES];
        random_release(customer_id, &seed, release);
        release_resources(customer_id, release);
        
        sleep(next_random(&seed) % 2 + 1);
    }
    
    // Release all remaining resources
    int release_all[NUM_RESOURCES];
    all_allocated_resources(customer_id, release_all);
    release_resources(customer_id, release_all);
    
    printf("Customer %d finished\n", customer_id);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != NUM_RESOURCES + 1) {
        fprintf(stderr, "Usage: %s <res1> <res2> <res3>\n", argv[0]);
        fprintf(stderr, "Example: %s 10 5 7\n", argv[0]);
        return 1;
    }
    
    // Parse available resources
    for (int i = 0; i < NUM_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
        if (available[i] <= 0) {
            fprintf(stderr, "Resource counts must be positive integers\n");
            return 1;
        }
    }
    
    unsigned int seed = (unsigned int)time(NULL);
    
    printf("Banker's Algorithm - Deadlock Avoidance\n");
    printf("=======================================\n");
    printf("Available resources: ");
    for (int i = 0; i < NUM_RESOURCES; i++) {
        printf("%d ", available[i]);
    }
    printf("\n\n");
    fflush(stdout);
    
    // Initialize maximum need for each customer
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        for (int j = 0; j < NUM_RESOURCES; j++) {
            maximum[i][j] = (int)(next_random(&seed) % (unsigned int)available[j]) + 1;
            need[i][j] = maximum[i][j];
            allocated[i][j] = 0;
        }
    }
    
    // Print maximum resources needed
    printf("Maximum resources needed by each customer:\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d: ", i);
        for (int j = 0; j < NUM_RESOURCES; j++) {
            printf("%d ", maximum[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    fflush(stdout);
    
    // Create customer threads
    pthread_t threads[NUM_CUSTOMERS];
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[i], NULL, customer, id) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all customers to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print statistics
    printf("\n===== Final Statistics =====\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        printf("Customer %d: %d requests, %d granted\n", i, request_count[i], grant_count[i]);
    }
    
    printf("Final available resources: ");
    for (int i = 0; i < NUM_RESOURCES; i++) {
        printf("%d ", available[i]);
    }
    printf("\n");
    
    // Cleanup
    pthread_mutex_destroy(&banker_lock);
    
    return 0;
}
