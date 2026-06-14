# HW4 Individual Programming Problems

## Overview
Two individual C programming problems for page replacement and disk scheduling.

## Problems

### 10.44* - Page-Replacement Algorithms
Implements FIFO, LRU, and optimal (OPT) page-replacement algorithms.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o page_replacement 10.44_page_replacement.c
```

**Usage:**
```bash
./page_replacement <number_of_frames> [reference_length] [seed]
```

**Example:**
```bash
./page_replacement 3
./page_replacement 3 20 12345
```

**Key Features:**
- Generates a random page-reference string with page numbers from 0 to 9
- Uses the same reference string for FIFO, LRU, and OPT comparison
- Reports the number of page faults for each algorithm
- Accepts the number of page frames from the command line
- Optional reference length and seed for repeatable testing

---

### 11.27* - Disk-Scheduling Algorithms
Implements FCFS, SCAN, and C-SCAN disk-scheduling algorithms.

**Compilation:**
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o disk_scheduling 11.27_disk_scheduling.c
```

**Usage:**
```bash
./disk_scheduling <initial_head_position> [seed]
```

**Example:**
```bash
./disk_scheduling 2150
./disk_scheduling 2150 12345
```

**Key Features:**
- Services a disk with 5,000 cylinders numbered 0 to 4,999
- Generates 1,000 random cylinder requests
- Uses the same request sequence for FCFS, SCAN, and C-SCAN comparison
- Reports the total amount of head movement for each algorithm
- Optional seed for repeatable testing

---

## Compilation Summary

Compile all individual problems:
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o page_replacement 10.44_page_replacement.c
gcc -Wall -Wextra -pedantic -std=c11 -o disk_scheduling 11.27_disk_scheduling.c
```

## Requirements
- Linux/Unix environment
- GCC compiler
- Standard C library

## Notes
- SCAN and C-SCAN initially move toward larger cylinder numbers.
- The optional seed is useful when recording screenshots or comparing output.
