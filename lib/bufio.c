#include "bufio.h"

struct buf_t {
    size_t capacity;
    size_t size;
};

struct buf_t *buf_new(size_t capacity) {
    struct buf_t *p = malloc(2 * sizeof(size_t) + capacity);
    p->capacity = capacity;
    p->size = 0;
    return p;
}

void buf_free(struct buf_t * p) {
    free(p);
}

size_t buf_capacity(struct buf_t * p) {
    #ifdef DEBUG
        if (buf == NULL) {
            abort();
        }
    #endif
    return p->capacity;
}

size_t buf_size(struct buf_t * p) {
    #ifdef DEBUG
        if (buf == NULL) {
            abort();
        }
    #endif
    return p->size;
}

ssize_t buf_fill(fd_t fd, struct buf_t *buf, size_t required) {
    #ifdef DEBUG
        if (buf == NULL || buf->capacity < required) {
            abort();
        }
    #endif
    char* buffer = (char*)buf + 2 * sizeof(size_t);
    ssize_t read_size;
    while (1) {
        read_size = read(fd, buffer + buf->size, required - buf->size);
        if (read_size == -1) {
            return -1;
        }
        buf->size += read_size;
        if (read_size == 0 || buf->size >= required) {
            return buf->size;
        }
    }
}

ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required) {
    #ifdef DEBUG
        if (buf == NULL) {
            abort();
        }
    #endif
    char* buffer = (char*)buf + 2 * sizeof(size_t);
    ssize_t write_size;
    ssize_t written = 0;
    while (1) {
        write_size = write(fd, buffer + written, buf->size - written);
        if (write_size == -1) {
            return -1;
        }
        written += write_size;
        if (written >= required || buf->size == written) {
            int i;
            for (i = 0; i < buf->size - written; i++) {
                buf[i] = buf[i + written];
            }
            buf->size -= written;
            return written;
        }
    }
}
