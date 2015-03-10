#include "helpers.h"

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
