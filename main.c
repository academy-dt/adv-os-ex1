#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#define LOG(fmt, ...) fprintf(stdin, fmt "\n", ##__VA_ARGS__)

void die(const char* msg)
{
    int err = errno;
    perror(msg);
    exit(err);
}

void run_child(char *argv)
{
    char *file = argv;

    unsigned argc = 0;
    char *args[128] = { 0 };
    args[argc++] = file;

    char *delim = file;
    while ((delim = strchr(delim, ' '))) {
        *delim = '\0';
        args[argc++] = ++delim;
    }

    execvp(file, args);
    // We should never reach this point
    die("Exec failed");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        LOG("Usage: %s (cmd)...", argv[0]);
        return ENOENT;
    }

    if (argc == 2) {
        run_child(argv[1]);
    }

    pid_t *children = malloc(argc * sizeof(pid_t));
    if (!children) {
        die("Malloc children failed");
    }
    memset(children, 0, argc * sizeof(pid_t));

    int next_in;
    int in = STDIN_FILENO;
    int out;

    for (int i = 0; i < argc; ++i) {
        if (i < argc - 1) {
            int pipefd[2];
            if (pipe2(pipefd, O_CLOEXEC) < 0) {
                die("Pipe failed");
            }
            next_in = pipefd[0];
            out = pipefd[1];
        } else {
            next_in = -1;
            out = STDOUT_FILENO;
        }

        pid_t pid = fork();
        if (pid < 0) {
            die("Fork failed");
        }

        if (pid == 0) { // Running as child
            if (dup2(in, STDIN_FILENO) < 0) {
                die("Dup stdin failed");
            }

            if (dup2(out, STDOUT_FILENO) < 0) {
                die("Dup stdout failed");
            }

            run_child(argv[i]);
        }

        if (in != STDIN_FILENO) {
            close(in);
        }

        if (out != STDOUT_FILENO) {
            close(out);
        }

        children[i] = pid;
        in = next_in;
    }

    for (int i = 0; i < argc; ++i) {
        int status;
        if (waitpid(children[i], &status, 0) != children[i]) {
            die("Wait for child failed");
        }
    }

    free(children);
    return 0;
}

