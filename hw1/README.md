# FileCopy - Exercise 2.24 Submission

## Program Description
A file copying program that takes source and destination filenames as command-line arguments and copies the file contents with comprehensive error checking.

## Compilation & Execution
```bash
gcc -o FileCopy FileCopy.c
./FileCopy <source_file> <destination_file>
```

## System Call Tracing
```bash
strace ./FileCopy source.txt dest.txt
```

## Features Implemented
- Command-line argument validation
- Source file existence and type verification
- Same file detection (prevent overwriting source)
- POSIX API for file I/O (open, read, write, close)
- Buffer-based copying (4KB chunks)
- Comprehensive error handling with perror()
- Proper file descriptor cleanup

## Collatz Conjecture - Exercise 3.21 Submission

### Program Description
A program that implements the Collatz conjecture (also known as the 3n+1 problem). It takes a positive integer as a command-line argument and computes the sequence by repeatedly applying the rules: if the number is even, divide by 2; if odd, multiply by 3 and add 1, until reaching 1. The program outputs the sequence and the number of steps.

### Compilation & Execution
```bash
gcc -o collatz collatz.c
./collatz <positive_integer>
```