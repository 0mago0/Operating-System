/**
 * Chapter 9 Project - Contiguous Memory Allocation
 * 
 * Implements memory allocation using different strategies:
 * - First-fit: Allocate from first available hole
 * - Best-fit: Allocate from smallest sufficient hole
 * - Worst-fit: Allocate from largest hole
 * 
 * Supports: allocate, release, compact, report operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_PROCESSES 100
#define DEFAULT_MEMORY_KB 65536

// Memory block structure
typedef struct {
    int start_address;
    int size;
    int process_id;    // -1 for free, process_id >= 0 for allocated
    bool is_free;
} memory_block_t;

// Global state
memory_block_t memory_map[MAX_PROCESSES];
int num_blocks = 1;
int total_memory = DEFAULT_MEMORY_KB * 1024;
int allocated_memory = 0;

// Statistics
int allocation_count = 0;
int release_count = 0;
int compact_count = 0;

/**
 * Initialize memory - all free initially
 */
void memory_init(int max_size) {
    total_memory = max_size;
    allocated_memory = 0;
    num_blocks = 1;
    
    memory_map[0].start_address = 0;
    memory_map[0].size = max_size;
    memory_map[0].process_id = -1;
    memory_map[0].is_free = true;
}

/**
 * Print current memory state
 */
void report_memory(void) {
    printf("\n========== Memory State ==========\n");
    printf("Total memory: %d KB\n", total_memory / 1024);
    printf("Allocated: %d KB (%.1f%%)\n", allocated_memory / 1024, 
           (allocated_memory * 100.0) / total_memory);
    printf("Free: %d KB (%.1f%%)\n", (total_memory - allocated_memory) / 1024,
           ((total_memory - allocated_memory) * 100.0) / total_memory);
    printf("\nMemory Map:\n");
    printf("%-5s %-10s %-10s %-10s %-10s\n", "Addr", "Size", "PID", "Type", "Status");
    printf("----------------------------------------------------------\n");
    
    for (int i = 0; i < num_blocks; i++) {
        const char* type = memory_map[i].is_free ? "FREE" : "ALLOCATED";
        printf("%-5d %-10d %-10d %-10s %-10s\n",
               memory_map[i].start_address,
               memory_map[i].size,
               memory_map[i].process_id,
               type,
               "OK");
    }
    printf("\n");
}

/**
 * Split a free block and allocate the requested portion.
 */
int allocate_from_block(int index, int size, int process_id, const char* strategy) {
    int original_start = memory_map[index].start_address;
    int original_size = memory_map[index].size;
    
    if (original_size == size) {
        memory_map[index].is_free = false;
        memory_map[index].process_id = process_id;
        allocated_memory += size;
        allocation_count++;
        printf("Process %d: ALLOCATED %d KB at address %d (%s, Exact)\n",
               process_id, size / 1024, original_start, strategy);
        return original_start;
    }
    
    if (num_blocks >= MAX_PROCESSES) {
        printf("Process %d: ALLOCATION FAILED (memory map full)\n", process_id);
        return -1;
    }
    
    for (int j = num_blocks; j > index + 1; j--) {
        memory_map[j] = memory_map[j - 1];
    }
    
    memory_map[index].start_address = original_start;
    memory_map[index].size = size;
    memory_map[index].is_free = false;
    memory_map[index].process_id = process_id;
    
    memory_map[index + 1].start_address = original_start + size;
    memory_map[index + 1].size = original_size - size;
    memory_map[index + 1].is_free = true;
    memory_map[index + 1].process_id = -1;
    
    num_blocks++;
    allocated_memory += size;
    allocation_count++;
    printf("Process %d: ALLOCATED %d KB at address %d (%s)\n",
           process_id, size / 1024, original_start, strategy);
    return original_start;
}

/**
 * First-Fit allocation strategy
 */
int allocate_first_fit(int size, int process_id) {
    if (size <= 0) {
        printf("Process %d: ALLOCATION FAILED (size must be positive)\n", process_id);
        return -1;
    }
    
    // Find first hole that fits
    for (int i = 0; i < num_blocks; i++) {
        if (memory_map[i].is_free && memory_map[i].size >= size) {
            return allocate_from_block(i, size, process_id, "First-Fit");
        }
    }
    
    printf("Process %d: ALLOCATION FAILED (insufficient contiguous memory)\n", process_id);
    return -1;
}

/**
 * Best-Fit allocation strategy
 */
int allocate_best_fit(int size, int process_id) {
    int best_index = -1;
    int best_size = INT_MAX;
    
    if (size <= 0) {
        printf("Process %d: ALLOCATION FAILED (size must be positive)\n", process_id);
        return -1;
    }
    
    // Find smallest hole that fits
    for (int i = 0; i < num_blocks; i++) {
        if (memory_map[i].is_free && memory_map[i].size >= size) {
            if (memory_map[i].size < best_size) {
                best_size = memory_map[i].size;
                best_index = i;
            }
        }
    }
    
    if (best_index == -1) {
        printf("Process %d: ALLOCATION FAILED (insufficient contiguous memory)\n", process_id);
        return -1;
    }
    
    return allocate_from_block(best_index, size, process_id, "Best-Fit");
}

/**
 * Worst-Fit allocation strategy
 */
int allocate_worst_fit(int size, int process_id) {
    int worst_index = -1;
    int worst_size = -1;
    
    if (size <= 0) {
        printf("Process %d: ALLOCATION FAILED (size must be positive)\n", process_id);
        return -1;
    }
    
    // Find largest hole that fits
    for (int i = 0; i < num_blocks; i++) {
        if (memory_map[i].is_free && memory_map[i].size >= size) {
            if (memory_map[i].size > worst_size) {
                worst_size = memory_map[i].size;
                worst_index = i;
            }
        }
    }
    
    if (worst_index == -1) {
        printf("Process %d: ALLOCATION FAILED (insufficient contiguous memory)\n", process_id);
        return -1;
    }
    
    return allocate_from_block(worst_index, size, process_id, "Worst-Fit");
}

/**
 * Release memory allocation
 */
bool release_memory(int process_id) {
    bool found = false;
    
    for (int i = 0; i < num_blocks; i++) {
        if (!memory_map[i].is_free && memory_map[i].process_id == process_id) {
            printf("Process %d: RELEASED %d KB from address %d\n",
                   process_id, memory_map[i].size / 1024, memory_map[i].start_address);
            
            allocated_memory -= memory_map[i].size;
            memory_map[i].is_free = true;
            memory_map[i].process_id = -1;
            found = true;
            release_count++;
        }
    }
    
    if (!found) {
        printf("Process %d: RELEASE FAILED (process not found)\n", process_id);
        return false;
    }
    
    return true;
}

/**
 * Compact memory - merge free blocks
 */
void compact_memory(void) {
    printf("\nCompacting memory...\n");
    
    // Move all allocated blocks to the beginning
    int allocated_pos = 0;
    
    for (int i = 0; i < num_blocks; i++) {
        if (!memory_map[i].is_free) {
            if (allocated_pos < i) {
                memory_map[allocated_pos] = memory_map[i];
                memory_map[allocated_pos].start_address = (allocated_pos == 0) ? 0 : 
                    memory_map[allocated_pos - 1].start_address + memory_map[allocated_pos - 1].size;
            }
            allocated_pos++;
        }
    }
    
    // Create single free block for remaining memory
    if (allocated_pos < num_blocks) {
        memory_map[allocated_pos].start_address = allocated_memory;
        memory_map[allocated_pos].size = total_memory - allocated_memory;
        memory_map[allocated_pos].is_free = true;
        memory_map[allocated_pos].process_id = -1;
        num_blocks = allocated_pos + 1;
    } else {
        num_blocks = allocated_pos;
    }
    
    compact_count++;
    printf("Compaction complete. Blocks reduced to %d\n", num_blocks);
}

/**
 * Interactive menu
 */
void show_menu(void) {
    printf("\n========== Memory Management System ==========\n");
    printf("1. Allocate (First-Fit)\n");
    printf("2. Allocate (Best-Fit)\n");
    printf("3. Allocate (Worst-Fit)\n");
    printf("4. Release\n");
    printf("5. Compact\n");
    printf("6. Report\n");
    printf("7. Exit\n");
    printf("=============================================\n");
}

int main(int argc, char* argv[]) {
    int max_size = DEFAULT_MEMORY_KB * 1024;
    
    if (argc > 1) {
        max_size = atoi(argv[1]) * 1024;  // Convert KB to bytes
        if (max_size <= 0) {
            fprintf(stderr, "Invalid memory size: must be a positive integer in KB\n");
            return 1;
        }
    }
    
    memory_init(max_size);
    
    printf("Contiguous Memory Allocation Manager\n");
    printf("====================================\n");
    printf("Total Memory: %d KB\n\n", max_size / 1024);
    
    int choice;
    int process_id = 1;
    
    while (1) {
        show_menu();
        printf("Enter choice (1-7): ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                printf("Enter process ID: ");
                scanf("%d", &process_id);
                printf("Enter size (KB): ");
                int size_kb;
                scanf("%d", &size_kb);
                allocate_first_fit(size_kb * 1024, process_id);
                break;
            }
            case 2: {
                printf("Enter process ID: ");
                scanf("%d", &process_id);
                printf("Enter size (KB): ");
                int size_kb;
                scanf("%d", &size_kb);
                allocate_best_fit(size_kb * 1024, process_id);
                break;
            }
            case 3: {
                printf("Enter process ID: ");
                scanf("%d", &process_id);
                printf("Enter size (KB): ");
                int size_kb;
                scanf("%d", &size_kb);
                allocate_worst_fit(size_kb * 1024, process_id);
                break;
            }
            case 4: {
                printf("Enter process ID to release: ");
                scanf("%d", &process_id);
                release_memory(process_id);
                break;
            }
            case 5: {
                compact_memory();
                break;
            }
            case 6: {
                report_memory();
                break;
            }
            case 7: {
                printf("\nFinal Statistics:\n");
                printf("Allocations: %d\n", allocation_count);
                printf("Releases: %d\n", release_count);
                printf("Compactions: %d\n", compact_count);
                printf("Goodbye!\n");
                return 0;
            }
            default:
                printf("Invalid choice!\n");
        }
    }
    
    return 0;
}
