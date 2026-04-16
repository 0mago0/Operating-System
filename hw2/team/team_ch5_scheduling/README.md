# Team Project - Chapter 5 Scheduling Algorithms

## Program Description
This project implements several CPU scheduling algorithms in C:

- First-Come, First-Served (FCFS)
- Shortest Job First (SJF)
- Priority scheduling
- Round Robin
- Priority with Round Robin

The program uses a built-in set of sample processes with arrival time, burst time, and priority. Lower priority numbers represent higher priority.

## Compilation & Execution
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o scheduling scheduling.c
./scheduling
./scheduling 3
```

The optional argument is the time quantum used by Round Robin and Priority with Round Robin.

## Output
For each algorithm, the program prints:

- A Gantt chart
- Completion time for each process
- Turnaround time for each process
- Waiting time for each process
- Average turnaround time
- Average waiting time

## Team Responsibility
- FCFS scheduling implementation
- SJF scheduling implementation
- Priority scheduling implementation
- Round Robin and Priority with Round Robin implementation
- Testing, output formatting, and documentation
