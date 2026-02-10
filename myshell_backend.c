#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_OUT 8192
#define MAX_ARGS 100
#define MAX_CMDS 10

/* ---------- SIGCHLD HANDLER ---------- */
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* ---------- BACKEND EXECUTION ---------- */
int execute_command_backend(char *command, char *output) {
    char *cmds[MAX_CMDS];
    int cmd_count = 0;
    int background = 0;
    char cmd_copy[1024];

    memset(output, 0, MAX_OUT);
    strcpy(cmd_copy, command);

    /* ---------- Detect background & ---------- */
    if (strchr(cmd_copy, '&')) {
        background = 1;
        cmd_copy[strcspn(cmd_copy, "&")] = '\0';
    }

    /* ---------- Split by pipe ---------- */
    char *token = strtok(cmd_copy, "|");
    while (token && cmd_count < MAX_CMDS) {
        cmds[cmd_count++] = token;
        token = strtok(NULL, "|");
    }

    int prev_fd = -1;
    int pipefd[2];
    int out_pipe[2];
    pid_t pids[MAX_CMDS];

    pipe(out_pipe);

    for (int i = 0; i < cmd_count; i++) {
        if (i < cmd_count - 1)
            pipe(pipefd);

        pid_t pid = fork();

        if (pid == 0) {
            /* ---------- CHILD ---------- */

            if (prev_fd != -1) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (i < cmd_count - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            } else {
                dup2(out_pipe[1], STDOUT_FILENO);
                dup2(out_pipe[1], STDERR_FILENO);
            }

            /* ---------- Parse args + redirections ---------- */
            char *args[MAX_ARGS];
            int j = 0;
            char *arg = strtok(cmds[i], " ");

            char *infile = NULL;
            char *outfile = NULL;
            int append = 0;

            while (arg && j < MAX_ARGS - 1) {
                if (strcmp(arg, "<") == 0) {
                    arg = strtok(NULL, " ");
                    infile = arg;
                } else if (strcmp(arg, ">") == 0) {
                    arg = strtok(NULL, " ");
                    outfile = arg;
                    append = 0;
                } else if (strcmp(arg, ">>") == 0) {
                    arg = strtok(NULL, " ");
                    outfile = arg;
                    append = 1;
                } else {
                    args[j++] = arg;
                }
                arg = strtok(NULL, " ");
            }
            args[j] = NULL;

            /* ---------- Input redirection ---------- */
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) {
                    perror("input redirection error");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            /* ---------- Output redirection ---------- */
            if (outfile) {
                int fd;
                if (append)
                    fd = open(outfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
                else
                    fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

                if (fd < 0) {
                    perror("output redirection error");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        }

        pids[i] = pid;

        if (prev_fd != -1)
            close(prev_fd);

        if (i < cmd_count - 1) {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }

    close(out_pipe[1]);

    if (background) {
        snprintf(output, MAX_OUT,
                 "[Background PID: %d]", pids[cmd_count - 1]);
        return 0;
    }

    read(out_pipe[0], output, MAX_OUT - 1);
    close(out_pipe[0]);

    for (int i = 0; i < cmd_count; i++)
        waitpid(pids[i], NULL, 0);

    return 0;
}

/* ---------- INIT ---------- */
__attribute__((constructor))
void init_backend() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
}
