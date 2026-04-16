# Team Project - Chapter 4 Sudoku Solution Validator

## Program Description
This project implements the Chapter 4 Sudoku solution validator. The program creates 27 POSIX threads:

- 9 threads validate the rows.
- 9 threads validate the columns.
- 9 threads validate the 3x3 subgrids.

Each thread records whether its assigned section contains every number from 1 to 9 exactly once. The parent thread joins all workers and prints whether the Sudoku solution is valid.

## Compilation & Execution
```bash
gcc -Wall -Wextra -pedantic -std=c11 -o sudoku_validator sudoku_validator.c -pthread
./sudoku_validator
```

The program includes a valid default puzzle. You may also provide all 81 cell values on the command line:

```bash
./sudoku_validator 6 2 4 5 3 9 1 8 7 5 1 9 7 2 8 6 3 4 8 3 7 6 1 4 2 9 5 1 4 3 8 6 5 7 2 9 9 5 8 2 4 7 3 6 1 7 6 2 3 9 1 4 5 8 3 7 1 9 5 6 8 4 2 4 9 6 1 8 2 5 7 3 2 8 5 4 7 3 9 1 6
```

## Output
The program prints the Sudoku grid and then reports:

- `The Sudoku solution is valid.`
- Or the invalid rows, columns, and subgrids followed by `The Sudoku solution is invalid.`

## Team Responsibility
- Passing row, column, and subgrid parameters to worker threads
- Validating all 27 Sudoku sections concurrently
- Returning validation results to the parent thread
- Testing, output formatting, and documentation
