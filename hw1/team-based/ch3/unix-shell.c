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
    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return i;
}

// Parses args for < and >, opens files, and updates in_fd/out_fd.
// Returns 0 on success, -1 on error.
// SIDE EFFECT: Modifies args array by inserting NULL at redirection points.
int handle_redirection(char **args, int *in_fd, int *out_fd) {
    *in_fd = -1;
    *out_fd = -1;
    
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
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

        if (strcmp(input_buffer, "!!") == 0) {
            if (history_count == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input_buffer, history[history_count - 1]);
            printf("%s\n", input_buffer);
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
            *pipe_pos = 0; 
            char *cmd1_str = input_buffer;
            char *cmd2_str = pipe_pos + 1;

            char *args1[MAX_LINE/2+1];
            char *args2[MAX_LINE/2+1];

            parse_line(cmd1_str, args1);
            parse_line(cmd2_str, args2);

            if (args1[0] == NULL || args2[0] == NULL) {
                printf("Invalid pipe command syntax.\n");
                continue;
            }

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                continue;
            }

            pid_t p1 = fork();
            if (p1 == 0) {
                close(pipefd[0]);
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) { perror("dup2 p1"); exit(1); }
                close(pipefd[1]);

                int in_fd=-1, out_fd=-1;
                if (handle_redirection(args1, &in_fd, &out_fd) < 0) exit(1);
                
                if (in_fd != -1) {
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                }
                if (out_fd != -1) {
                    dup2(out_fd, STDOUT_FILENO);
                    close(out_fd);
                }

                if (execvp(args1[0], args1) < 0) {
                    fprintf(stderr, "Command not found: %s\n", args1[0]);
                    exit(1);
                }
            }
            
            pid_t p2 = fork();
            if (p2 == 0) {
                close(pipefd[1]);
                if (dup2(pipefd[0], STDIN_FILENO) < 0) { perror("dup2 p2"); exit(1); }
                close(pipefd[0]);

                int in_fd=-1, out_fd=-1;
                if (handle_redirection(args2, &in_fd, &out_fd) < 0) exit(1);

                if (in_fd != -1) {
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                }
                if (out_fd != -1) {
                    dup2(out_fd, STDOUT_FILENO);
                    close(out_fd);
                }

                if (execvp(args2[0], args2) < 0) {
                    fprintf(stderr, "Command not found: %s\n", args2[0]);
                    exit(1);
                }
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
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
