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

void run_child(char *argv0)
{
    char *file = argv0;

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

    int pipefd[2];
    if (pipe2(pipefd, O_CLOEXEC) < 0) {
        die("Pipe failed");
    }

    pid_t pid = fork();
    if (pid < 0) {
        die("Fork failed");
    }

    if (pid == 0) { // Running as child
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            die("Dup stdout failed");
        }
        run_child(argv[1]);
    }

    // Running as parent

    int status;
    if (waitpid(pid, &status, 0) != pid) {
        die("Wait for child failed");
    }

    if (dup2(pipefd[0], STDIN_FILENO) < 0) {
        die("Dup stdin failed");
    }

    argv[1] = argv[0]; // Skip child command we just ran
    execvp(argv[1], &argv[1]);
    // We should never reach this point
    die("Exec failed");

    return 0;
}

