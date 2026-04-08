#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_LINE 80 /* The maximum length command */
#define HISTORY_SIZE 10

char *history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char *cmd) {
    if (history_count < HISTORY_SIZE) {
        history[history_count] = strdup(cmd);
        history_count++;
    } else {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = strdup(cmd);
    }
}

void show_history() {
    for (int i = history_count - 1; i >= 0; i--) {
        printf("%d %s\n", history_count - i, history[i]);
    }
}

int parse_line(char *input, char **args) {
    int arg_idx = 0;
    int arg_len = 0;
    char current_arg[MAX_LINE+1];
    int in_quotes = 0;
    
    // Parse directly from input, handling quotes properly
    for (int k = 0; input[k] && arg_idx < MAX_LINE/2; k++) {
        if (input[k] == '"') {
            in_quotes = !in_quotes;  // Toggle quote state, skip the quote itself
        } else if ((input[k] == ' ' || input[k] == '\t' || input[k] == '\n') && !in_quotes) {
            if (arg_len > 0) {
                current_arg[arg_len] = '\0';
                args[arg_idx++] = strdup(current_arg);
                arg_len = 0;
            }
        } else {
            if (arg_len < MAX_LINE) {
                current_arg[arg_len++] = input[k];
            }
        }
    }
    
    // Add last argument if exists
    if (arg_len > 0) {
        current_arg[arg_len] = '\0';
        args[arg_idx++] = strdup(current_arg);
    }
    
    args[arg_idx] = NULL;
    return arg_idx;
}

// Parses args for <, >, and >>, opens files, and updates in_fd/out_fd.
// Returns 0 on success, -1 on error.
// SIDE EFFECT: Modifies args array by inserting NULL at redirection points.
int handle_redirection(char **args, int *in_fd, int *out_fd) {
    *in_fd = -1;
    *out_fd = -1;
    
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">>") == 0) {
            if (args[i+1] != NULL) {
                *out_fd = open(args[i+1], O_CREAT | O_APPEND | O_WRONLY, 0644);
                if (*out_fd < 0) {
                    perror("open output file");
                    return -1;
                }
                args[i] = NULL; // Terminate command args here
            } else {
                fprintf(stderr, "Missing output file for redirection >>\n");
                return -1;
            }
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i+1] != NULL) {
                *out_fd = open(args[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
                if (*out_fd < 0) {
                    perror("open output file");
                    return -1;
                }
                args[i] = NULL; // Terminate command args here
            } else {
                fprintf(stderr, "Missing output file for redirection >\n");
                return -1;
            }
        } else if (strcmp(args[i], "<") == 0) {
            if (args[i+1] != NULL) {
                *in_fd = open(args[i+1], O_RDONLY);
                if (*in_fd < 0) {
                    perror("open input file");
                    return -1;
                }
                args[i] = NULL; // Terminate command args here
            } else {
                fprintf(stderr, "Missing input file for redirection <\n");
                return -1;
            }
        }
    }
    return 0;
}

void execute_command(char **args, int background, int in_fd, int out_fd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    
    if (pid == 0) {
        // Child process
        if (in_fd != -1) {
            if (dup2(in_fd, STDIN_FILENO) == -1) {
                perror("dup2 input");
                exit(1);
            }
            close(in_fd);
        }
        if (out_fd != -1) {
            if (dup2(out_fd, STDOUT_FILENO) == -1) {
                perror("dup2 output");
                exit(1);
            }
            close(out_fd);
        }
        
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "Command not found: %s\n", args[0]);
            exit(1);
        }
    } else {
        // Parent process
        if (!background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("[Process running in background with PID %d]\n", pid);
        }
    }
}

int main(void) {
    char input_buffer[MAX_LINE+1];
    int should_run = 1;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        if (fgets(input_buffer, MAX_LINE, stdin) == NULL) {
            break; 
        }

        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strlen(input_buffer) == 0) continue;

        // Handle history commands: !!, !n
        if (input_buffer[0] == '!') {
            if (strcmp(input_buffer, "!!") == 0) {
                if (history_count == 0) {
                    printf("No commands in history.\n");
                    continue;
                }
                strcpy(input_buffer, history[history_count - 1]);
                printf("%s\n", input_buffer);
            } else if (isdigit(input_buffer[1])) {
                // Handle !n syntax (e.g., !1, !2)
                int cmd_num = atoi(&input_buffer[1]);
                if (cmd_num <= 0 || cmd_num > history_count) {
                    printf("History index out of range.\n");
                    continue;
                }
                strcpy(input_buffer, history[cmd_num - 1]);
                printf("%s\n", input_buffer);
            } else {
                printf("Invalid history command.\n");
                continue;
            }
        }

        add_to_history(input_buffer);

        if (strcmp(input_buffer, "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (strcmp(input_buffer, "history") == 0) {
            show_history();
            continue;
        }

        char *pipe_pos = strchr(input_buffer, '|');
        if (pipe_pos != NULL) {
            // Parse all piped commands
            char *cmd_str[MAX_LINE/2+1];
            int cmd_count = 0;
            char *copy = strdup(input_buffer);
            char *saveptr;
            char *token = strtok_r(copy, "|", &saveptr);
            
            while (token != NULL && cmd_count < MAX_LINE/2) {
                // Trim spaces
                while (*token == ' ' || *token == '\t') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && (*end == ' ' || *end == '\t')) *end-- = '\0';
                cmd_str[cmd_count++] = strdup(token);
                token = strtok_r(NULL, "|", &saveptr);
            }
            free(copy);
            
            if (cmd_count < 2) {
                printf("Invalid pipe syntax.\n");
                continue;
            }
            
            // Create pipes
            int pipes[cmd_count-1][2];
            for (int i = 0; i < cmd_count - 1; i++) {
                if (pipe(pipes[i]) == -1) {
                    perror("pipe");
                    continue;
                }
            }
            
            // Fork processes
            pid_t pids[cmd_count];
            for (int i = 0; i < cmd_count; i++) {
                pids[i] = fork();
                if (pids[i] == 0) {
                    // Child process
                    char *args[MAX_LINE/2 + 1];
                    parse_line(cmd_str[i], args);
                    
                    // Redirect stdin from previous pipe or file
                    if (i > 0) {
                        dup2(pipes[i-1][0], STDIN_FILENO);
                    } else {
                        // First command: check for input redirection
                        int in_fd = -1, out_fd = -1;
                        handle_redirection(args, &in_fd, &out_fd);
                        if (in_fd != -1) {
                            dup2(in_fd, STDIN_FILENO);
                            close(in_fd);
                        }
                    }
                    
                    // Redirect stdout to next pipe or file
                    if (i < cmd_count - 1) {
                        dup2(pipes[i][1], STDOUT_FILENO);
                    } else {
                        // Last command: check for output redirection
                        int in_fd = -1, out_fd = -1;
                        handle_redirection(args, &in_fd, &out_fd);
                        if (out_fd != -1) {
                            dup2(out_fd, STDOUT_FILENO);
                            close(out_fd);
                        }
                    }
                    
                    // Close all pipes in child
                    for (int j = 0; j < cmd_count - 1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    
                    // Execute command
                    execvp(args[0], args);
                    fprintf(stderr, "Command not found: %s\n", args[0]);
                    exit(1);
                }
            }
            
            // Parent: close all pipes and wait
            for (int i = 0; i < cmd_count - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            
            for (int i = 0; i < cmd_count; i++) {
                waitpid(pids[i], NULL, 0);
            }
            
            for (int i = 0; i < cmd_count; i++) {
                free(cmd_str[i]);
            }
            continue;
        }

        char *args[MAX_LINE/2 + 1];
        int argc = parse_line(input_buffer, args);
        if (argc == 0) continue;

        int background = 0;
        if (strcmp(args[argc-1], "&") == 0) {
            background = 1;
            args[argc-1] = NULL;
            argc--;
        }

        int in_fd = -1, out_fd = -1;
        if (handle_redirection(args, &in_fd, &out_fd) < 0) {
            if (in_fd != -1) close(in_fd);
            if (out_fd != -1) close(out_fd);
            continue;
        }
        
        if (args[0] == NULL) {
            if (in_fd != -1) close(in_fd);
            if (out_fd != -1) close(out_fd);
            continue;
        }

        execute_command(args, background, in_fd, out_fd);

        if (in_fd != -1) close(in_fd);
        if (out_fd != -1) close(out_fd);
    }
    
    for(int i=0; i<history_count; i++) {
        free(history[i]);
    }

    return 0;
}
