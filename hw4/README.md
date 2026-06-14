# HW4 - Memory Management and Storage Management

Operating System Homework Assignment #4 (Due: Jun. 16, 2026)

## Structure

```text
hw4/
├── individual/
│   ├── 10.44_page_replacement.c
│   ├── 11.27_disk_scheduling.c
│   └── README.md
└── README.md
```

## Programming Problems

| # | Title | Concepts | Status |
|---|-------|----------|--------|
| 10.44* | Page-Replacement Algorithms | FIFO, LRU, OPT, page faults | Done |
| 11.27* | Disk-Scheduling Algorithms | FCFS, SCAN, C-SCAN, head movement | Done |

## Quick Start

```bash
cd hw4/individual
gcc -Wall -Wextra -pedantic -std=c11 -o page_replacement 10.44_page_replacement.c
gcc -Wall -Wextra -pedantic -std=c11 -o disk_scheduling 11.27_disk_scheduling.c

./page_replacement 3 20 12345
./disk_scheduling 2150 12345
```

## Notes
- Both programs generate random input data as required by the assignment.
- Both programs accept an optional seed so the same random data can be reproduced for testing.
