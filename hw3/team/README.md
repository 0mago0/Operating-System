# HW3 Team-based Programming Projects

## Project Selection
This submission includes three team projects:
- **Chapter 7**: The Dining Philosophers Problem  
- **Chapter 8**: Banker's Algorithm for Deadlock Avoidance
- **Chapter 9**: Contiguous Memory Allocation

(Note: Assignment requires 1 from Ch7 + 1 from Ch8&9, so either Ch8 OR Ch9 is needed)

---

## Chapter 7: The Dining Philosophers Problem

### Problem Description
Five philosophers sit at a round table. Between each pair of adjacent philosophers is a single fork. Each philosopher needs two forks (left and right) to eat. After eating, a philosopher thinks and may later want to eat again.

### Solution Approach
Uses POSIX mutex locks and condition variables with a "waiter" coordination strategy:
- Each philosopher has a state: THINKING, HUNGRY, or EATING
- A central mutex protects all state changes
- Condition variables allow philosophers to wait for availability
- Test function checks if both neighbors are not eating before allowing eating

### Compilation
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o dining_philosophers chap7_dining_philosophers.c
```

### Usage
```bash
./dining_philosophers
```

### Key Features
- ✓ Deadlock prevention
- ✓ POSIX threads (5 philosophers)
- ✓ Mutex locks for mutual exclusion
- ✓ Condition variables for coordination
- ✓ Tracks eating statistics
- ✓ No starvation during execution

### Output Example
```
Dining Philosophers Problem
===========================

[TIMESTAMP] Philosopher 0 thinking for 2 seconds
[TIMESTAMP] Philosopher 0 is hungry, waiting for forks...
[TIMESTAMP] Philosopher 0 picked up forks, starting to eat
[TIMESTAMP] Philosopher 0 finished eating, thinking
...
===== Final Statistics =====
Philosopher 0 ate 3 times
Philosopher 1 ate 3 times
...
```

---

## Chapter 8: Banker's Algorithm for Deadlock Avoidance

### Problem Description
Implements the Banker's Algorithm from Section 8.6.3 to prevent deadlock. Multiple customers request and release resources. The banker grants requests only if they lead to a safe state.

### Algorithm Overview
1. Each customer declares its maximum resource needs upfront
2. Customers request resources and release them over time
3. Banker uses safety algorithm to check if granting request maintains safe state:
   - Find sequence where each customer can complete with available resources
   - If safe sequence exists, grant request; otherwise, deny it

### Compilation
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bankers_algorithm chap8_bankers_algorithm.c
```

### Usage
```bash
./bankers_algorithm <resource1> <resource2> <resource3>
```

### Example
```bash
./bankers_algorithm 10 5 7
```

### Key Features
- ✓ 5 customer threads requesting resources
- ✓ 3 resource types
- ✓ Safety algorithm implementation
- ✓ Mutex protection for banker state
- ✓ Request denial if unsafe
- ✓ Statistics on grant/deny rates
- ✓ Deadlock avoidance guaranteed

### Data Structures
```c
int available[3]                    // Available resources of each type
int maximum[5][3]                   // Maximum need per customer
int allocated[5][3]                 // Currently allocated to customer
int need[5][3]                       // Remaining need (maximum - allocated)
```

### Output Example
```
Banker's Algorithm - Deadlock Avoidance
=======================================
Available resources: 10 5 7

Maximum resources needed by each customer:
Customer 0: 3 2 2
Customer 1: 2 2 2
...

Customer 0: Request GRANTED (resources: 1 0 2 )
Customer 1: Request DENIED (would lead to unsafe state)
Customer 2: Request GRANTED (resources: 0 1 1 )
...

===== Final Statistics =====
Customer 0: 3 requests, 2 granted
Customer 1: 3 requests, 1 granted
...
```

---

## Compilation Summary

```bash
# Compile Dining Philosophers
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o dining_philosophers chap7_dining_philosophers.c

# Compile Banker's Algorithm
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bankers_algorithm chap8_bankers_algorithm.c
```

---

## Requirements
- Linux/Unix with POSIX support
- GCC compiler with pthread support
- C standard library

## Design Choices

### Dining Philosophers
- **Approach**: Monitor/Waiter pattern with global state
- **Synchronization**: Mutex + condition variables
- **Fairness**: No explicit fairness guarantee (basic algorithm)
- **Scalability**: Fixed to 5 philosophers (classic problem)

### Banker's Algorithm
- **Safety Check**: Standard safety algorithm with finish array
- **Resource Types**: 3 types (easily configurable)
- **Customers**: 5 customers (easily configurable)
- **Thread Safety**: Mutex protects all shared data
- **Request Strategy**: Random requests within need limits

---

## Chapter 9: Contiguous Memory Allocation

### Problem Description
Implements memory allocation using three different strategies to manage a contiguous region of memory. Processes allocate and release memory blocks, and fragmentation can be managed through compaction.

### Allocation Strategies

| Strategy | Selection Criteria | Advantage | Disadvantage |
|----------|------------------|-----------|--------------|
| **First-Fit** | First hole large enough | Fast search | External fragmentation |
| **Best-Fit** | Smallest sufficient hole | Minimizes wasted space | Slow search, creates tiny holes |
| **Worst-Fit** | Largest available hole | Leaves larger holes for future | More fragmentation |

### Compilation
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o memory_allocation chap9_contiguous_memory.c
```

### Usage
```bash
./memory_allocation [SIZE_KB]
```

### Example
```bash
./memory_allocation 64    # 64 KB memory (default: 64 MB)
```

### Interactive Commands
```
1. Allocate (First-Fit)    - Request memory using first-fit
2. Allocate (Best-Fit)     - Request memory using best-fit
3. Allocate (Worst-Fit)    - Request memory using worst-fit
4. Release                 - Free a process's memory
5. Compact                 - Merge free holes
6. Report                  - Display memory state
7. Exit                    - Quit program
```

### Key Features
- ✓ Three allocation strategies (First, Best, Worst)
- ✓ Memory release with fragmentation handling
- ✓ Compaction to eliminate external fragmentation
- ✓ Interactive interface for testing
- ✓ Detailed memory map display
- ✓ Statistics tracking

### Output Example
```
========== Memory State ==========
Total memory: 64 KB
Allocated: 32 KB (50.0%)
Free: 32 KB (50.0%)

Memory Map:
Addr       Size       PID        Type       Status
---------------------------------------------------
0          16384      1          ALLOCATED  OK
16384      16384      2          ALLOCATED  OK
32768      32768      -1         FREE       OK

Process 1: ALLOCATED 16 KB at address 0 (First-Fit)
```

### Memory Management Concepts
- **External Fragmentation**: Free space split into holes too small for allocation
- **Compaction**: Moving allocated blocks to eliminate holes
- **Allocation Efficiency**: Balance between speed and space utilization
- **Process Lifecycle**: Allocate → Use → Release

---

## Compilation Summary

```bash
# Compile all Team projects
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o dining_philosophers chap7_dining_philosophers.c
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bankers_algorithm chap8_bankers_algorithm.c
gcc -Wall -Wextra -pedantic -std=c11 -o memory_allocation chap9_contiguous_memory.c
```

---

## Potential Extensions
- Add starvation prevention to Dining Philosophers
- Implement multiple rounds with different request patterns
- Add performance metrics (grant/deny ratios, average wait times)
- Support arbitrary number of philosophers and resources
- Add visualization of memory fragmentation over time
- Implement buddy system or other advanced allocation techniques
