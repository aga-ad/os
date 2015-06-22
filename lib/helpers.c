#define _GNU_SOURCE

#include "helpers.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t offset = 0;
    ssize_t size;
    while (1) {
        size = read(fd, buf + offset, count - offset);
        if (size == -1) {
            return -1;
        }
        if (size == 0) {
            return offset;
        }
        offset += size;
    }
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t offset = 0;
    ssize_t size;
    while (1) {
        size = write(fd, buf + offset, count - offset);
        if (size == -1) {
            return -1;
        }
        if (offset == count) {
            return offset;
        }
        offset += size;
    }
}

ssize_t read_until(int fd, void * buf, size_t count, char delimiter) {
    ssize_t offset = 0;
    ssize_t size;
    char* cbuf = (char*)buf;
    int i;
    while (1) {
        size = read(fd, buf + offset, count - offset);
        if (size == -1) {
            return -1;
        }
        if (size == 0) {
            return offset;
        }
        for (i = 0; i < size; i++) {
            if (*(cbuf + offset + i) == delimiter) {
                return offset + size;
            }
        }
        offset += size;
    }
}

int spawn(const char * file, char * const argv []) {
    int res = fork();
    if (res == 0) {
        exit(execvp(file, argv));
    }
    else if (res > 0) {
        int status;
        wait(&status);
        return status;
    }
    else {
        return -1;
    }
}


execargs_t new_execargs_t(int argc, char** argv) {
    execargs_t ret;
    int i;
    ret.argv = (char**) malloc((argc + 1) * sizeof(char*));
    for (i = 0; i < argc; i++)
        ret.argv[i] = strdup(argv[i]);
    ret.argv[argc] = NULL;
    return ret;
}


int exec(execargs_t* args) {
    return spawn(args->argv[0], args->argv);
}

int childn;
int* childa;

void sig_handler(int sig) {
    int i;
    for (i = 0; i < childn; i++) {
        kill(childa[i], SIGKILL);
        waitpid(childa[i], NULL, 0);

    }
    childn = 0;
}

int runpiped(execargs_t** programs, size_t n) {
    if (n == 0)
        return 0;
    int i;
    int pipefd[n - 1][2];
    if (pipefd == 0) {
        return 1;
    }
    int child[n];
    for (i = 0; i + 1 < n; i++) {
        if (pipe2(pipefd[i], O_CLOEXEC) < 0) {
            return -1;
        }
    }

    size_t j;
    for (i = 0; i < n; i++) {
        child[i] = fork();
        if (child[i] == 0) {
            if (i + 1 < n) {
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
            if (i > 0) {
                dup2(pipefd[i - 1][0], STDIN_FILENO);
            }
            _exit(execvp(programs[i]->argv[0], programs[i]->argv));
        }
        if (child[i] == -1) {
            for (j = 0; j < i; j++) {
                kill(child[j], SIGKILL);
                waitpid(child[j], NULL, 0);
            }
            return -1;
        }
    }
    for (i = 0; i + 1 < n; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    childn = n;
    childa = (int*)child;
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sig_handler;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        for (j = 0; j < n; j++) {
            kill(child[j], SIGKILL);
            waitpid(child[j], NULL, 0);
        }
        return -1;
    }

    int status;
    for (i = 0; i < n; i++) {
        waitpid(child[i], &status, 0);
    }
    childn = 0;
    return 0;
}


