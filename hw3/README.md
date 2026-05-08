# HW3 - Process Synchronization and Memory Management

Operating System Homework Assignment #3 (Due: May 12, 2026)

## Structure

```
hw3/
├── individual/          # Individual programming problems (4 required)
│   ├── 7.17_monte_carlo_pi.c
│   ├── 7.14_pid_manager_threads.c
│   ├── 8.32_bridge_problem.c
│   ├── 9.28_addresses.c
│   └── README.md
├── team/               # Team-based projects (select 1 from Ch7 + 1 from Ch8&9)
│   ├── chap7_dining_philosophers.c
│   ├── chap8_bankers_algorithm.c
│   └── README.md
└── README.md
```

## Assignment Overview

### Theory Problems (Written)
- Chap.7: 7.8
- Chap.8: 8.20, 8.28(a)(d), 8.30
- Chap.9: 9.13, 9.24

### Programming Problems (Individual - All Required)

| # | Title | Concepts | Status |
|---|-------|----------|--------|
| 7.17* | Monte Carlo π Estimation | Mutex locks, shared counters, multithreading | ✓ |
| 7.14** | PID Manager Thread Safety | Mutex, race conditions, resource management | ✓ |
| 8.32* | Bridge Problem (Deadlock Prevention) | Semaphores, signaling, deadlock avoidance | ✓ |
| 9.28* | Virtual Address Translation | Bit manipulation, paging | ✓ |

### Programming Projects (Team-based - Select 1 from Ch7 + 1 from Ch8&9)

**Chapter 7 Options** (Choose 1):
- ✓ **Project 3: Dining Philosophers Problem** - Classic synchronization problem
  - Other options: Thread Pool, Sleeping TA, Producer-Consumer
  
**Chapter 8 & 9 Options** (Choose 1):
- ✓ **Chap 8: Banker's Algorithm** - Deadlock avoidance with safety algorithm
- ✓ **Chap 9: Contiguous Memory Allocation** - Three allocation strategies

---

## Quick Start

### Compile Everything

```bash
# Individual programs
cd individual
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o monte_carlo_pi 7.17_monte_carlo_pi.c -lm
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o pid_manager 7.14_pid_manager_threads.c
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bridge 8.32_bridge_problem.c
gcc -Wall -Wextra -pedantic -std=c11 -o addresses 9.28_addresses.c

# Team projects
cd ../team
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o dining_philosophers chap7_dining_philosophers.c
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bankers_algorithm chap8_bankers_algorithm.c
gcc -Wall -Wextra -pedantic -std=c11 -o memory_allocation chap9_contiguous_memory.c
```

### Run Examples

```bash
# Individual problems
./monte_carlo_pi 4 1000000        # 4 threads, 1M points each
./pid_manager 5                    # 5 threads requesting PIDs
./bridge 3 3                       # 3 northbound, 3 southbound farmers
./addresses 19986                  # Virtual address conversion

# Team projects
./dining_philosophers              # 5 philosophers
./bankers_algorithm 10 5 7         # Resources: 10, 5, 7
./memory_allocation 64             # 64 KB memory (interactive)
```

---

## Key Concepts

### Synchronization Primitives Used
- **Mutex Locks** (`pthread_mutex_*`) - Mutual exclusion
- **Semaphores** (`sem_*`) - Counting and binary semaphores
- **Condition Variables** (`pthread_cond_*`) - Wait/signal coordination

### Deadlock Prevention Techniques
- **Ordering** - Linear resource ordering
- **Signaling** - Careful sequencing of wake-ups
- **Banker's Algorithm** - Safety checking before allocation
- **Waiter Pattern** - Centralized coordination

### Memory Management Concepts
- Virtual address translation (paging)
- Page tables and TLB
- Bit manipulation for address extraction

---

## Compilation Standards

All programs compiled with:
```bash
-Wall -Wextra -pedantic -std=c11 -pthread
```

Additional flags as needed:
- `-lm` for math library (Monte Carlo)
- `-lrt` for real-time functions (if used)

---

## Verification

Each program should:
1. Compile without warnings
2. Run without errors or race conditions
3. Demonstrate proper synchronization
4. Output meaningful results
5. Handle edge cases gracefully

---

## Performance Considerations

- **Scalability**: Programs tested with varying thread/resource counts
- **Fairness**: No thread/customer starvation (for most cases)
- **Safety**: Mutex and semaphore usage prevents race conditions
- **Efficiency**: Minimal busy-waiting or unnecessary locking

---

## References

- POSIX Threads: `man pthreads`, `man pthread_mutex_*`, `man sem_*`
- Safety Algorithm: Silberschatz, Galvin, Gagne "Operating System Concepts"
- Bit Manipulation: Powers of 2, masking operations

---

## Author Notes

This implementation demonstrates:
1. Proper use of POSIX synchronization primitives
2. Deadlock prevention techniques
3. Thread-safe resource management
4. Careful coordination in complex scenarios
5. Memory safety and error handling

Each program can be compiled and run independently for testing individual concepts.
