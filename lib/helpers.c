#include "helpers.h"
#include "stdio.h"


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
