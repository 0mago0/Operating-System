# HW3 Individual Programming Problems

## Overview
Four individual C programming problems related to process synchronization and memory management.

## Problems

### 7.17* - Multithreaded Monte Carlo π Estimation
Estimates π using Monte Carlo method with multiple threads and mutex lock protection.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o monte_carlo_pi 7.17_monte_carlo_pi.c -lm
```

**Usage:**
```bash
./monte_carlo_pi <num_threads> <points_per_thread>
```

**Example:**
```bash
./monte_carlo_pi 4 1000000
```

**Key Features:**
- Multiple threads generate random points concurrently
- Mutex lock protects shared counters
- Calculates π estimate from points within circle
- Thread-safe access to global variables

---

### 7.14** - PID Manager with Thread Safety (Optional)
Extends a PID manager to be thread-safe using mutex locks.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o pid_manager 7.14_pid_manager_threads.c
```

**Usage:**
```bash
./pid_manager <num_threads>
```

**Example:**
```bash
./pid_manager 5
```

**Key Features:**
- Allocates/releases unique PIDs in range [300, 5000]
- Mutex protects PID bitmap for thread-safe access
- Multiple threads request/release PIDs concurrently
- Reports available PIDs count

---

### 8.32* - Bridge Problem (Single-Lane Bridge Synchronization)
Prevents deadlock when farmers from both sides try to use a single-lane bridge.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bridge 8.32_bridge_problem.c
```

**Usage:**
```bash
./bridge <northbound_farmers> <southbound_farmers>
```

**Example:**
```bash
./bridge 3 3
```

**Key Features:**
- Semaphores control bridge access
- Binary semaphore for bridge mutual exclusion
- Signaling semaphores for northbound/southbound farmers
- Prevents deadlock by careful signaling
- Prefers same-direction farmers to reduce contention

---

### 9.28* - Virtual Address Translation
Converts a 32-bit virtual address to page number and offset for 4-KB pages.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o addresses 9.28_addresses.c
```

**Usage:**
```bash
./addresses <virtual_address_in_decimal>
```

**Example:**
```bash
./addresses 19986
```

**Output:**
```
The address 19986 contains:
page number=4
offset=3602
```

**Key Features:**
- 32-bit address space
- 4-KB (2^12 byte) page size
- Extracts 20-bit page number and 12-bit offset
- Uses bit masking for efficient extraction

---

## Compilation Summary

Compile all Individual problems:
```bash
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o monte_carlo_pi 7.17_monte_carlo_pi.c -lm
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o pid_manager 7.14_pid_manager_threads.c
gcc -Wall -Wextra -pedantic -std=c11 -pthread -o bridge 8.32_bridge_problem.c
gcc -Wall -Wextra -pedantic -std=c11 -o addresses 9.28_addresses.c
```

## Requirements
- Linux/Unix with POSIX support
- GCC compiler with pthread support
- Standard C library

## Notes
- All programs use POSIX threads and synchronization primitives
- Proper mutex and semaphore usage prevents race conditions
- Compilation flags: `-Wall -Wextra -pedantic -std=c11 -pthread`
- Link with `-lm` for math library (pi estimation) if needed
