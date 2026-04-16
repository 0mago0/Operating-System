# Team Project - Chapter 4 Multithreaded Sorting

## Program Description
This project implements the Chapter 4 multithreaded sorting application. A list of integers is divided into two smaller lists. Two worker threads sort the sublists, and a third worker thread merges the sorted sublists into the final sorted list.

The program uses POSIX pthreads and `qsort()` for the two sublist sorting steps.

## Compilation & Execution
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o multithreaded_sort multithreaded_sort.c -pthread
./multithreaded_sort
./multithreaded_sort 9 4 7 1 3 8 2 6
```

## Output
The program prints:

- The original list
- The sorted left half
- The sorted right half
- The final merged sorted list

## Team Responsibility
- Thread design and parameter passing
- Sorting each sublist in separate threads
- Merging sorted sublists in a third thread
- Compilation and documentation
