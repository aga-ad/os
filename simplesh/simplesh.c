#include "bufio.h"
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

const int BUF_SIZE = 4096;
const int ARGS_NUM = 2048;


void sigint_handler(int sig) {

}

int is_splitter(char c) {
    return c == '|' || c == '\0' || c == ' ' || c == '\t' || c == '\n';
}

int find_arguments(char* s) {
    if (*s == '\0')
        return 0;
    int count = 0;
    s++;
    while (1) {
        count += (!is_splitter(*(s - 1)) && is_splitter(*s));
        if (*s == '\0' || *s == '|') {
            return count;
        }
        s++;
    }
    return count;
}

char* get_arg(char* s, int* shift) {
    while (s[*shift] == ' ' || s[*shift] == '\t') {
        ++*shift;
    }
    int start = *shift;
    while (!is_splitter(s[*shift])) {
        ++*shift;
    }
    if (start == *shift) {
        return 0;
    } else {
        return strndup(s + start, *shift - start);
    }
}

void move_to_next(char* s, int* shift) {
    while (s[*shift] == ' ' || s[*shift] == '\t'){
        ++*shift;
    }
    if (s[*shift] == '|') {
        ++*shift;
    }
}

int main() {
    struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = &sigint_handler;

    if (sigaction(SIGINT, &sigact, NULL) < 0)
        return 1;

    struct buf_t* buf = buf_new(BUF_SIZE);
    char str[BUF_SIZE];
    int rc, i, shift, k, argc;
    while(1) {
        if (write_(STDOUT_FILENO, "$ ", 2) < 1)
            return 1;
        rc = buf_getline(STDIN_FILENO, buf, str);
        if (rc == 0) {
            return 0;
        }
        if (rc < 0) {
            if (write_(STDOUT_FILENO, "\n$ ", 2) < 1) {
                return 1;
            }
            continue;
        }
        str[rc] = '\0';
        execargs_t* arguments[ARGS_NUM];
        shift = 0;
        k = 0;
        while (1) {
            argc = find_arguments(str + shift);
            if (argc == 0) {
                break;
            }
            char* argv[argc];
            for (i = 0; i < argc; i++) {
                argv[i] = get_arg(str, &shift);
            }
            arguments[k] = (execargs_t*) malloc(sizeof(execargs_t));
            *arguments[k] = new_execargs_t(argc, argv);
            move_to_next(str, &shift);
            k++;
        }
        runpiped(arguments, k);
    }

    return 0;
}
