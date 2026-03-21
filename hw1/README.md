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