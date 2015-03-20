#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argn, char** args) {
    const size_t BUFFER_CAPACITY = 4096;
    char buf[BUFFER_CAPACITY];
    ssize_t read_size, buffer_size, delstr_size = strlen(args[1]);
    int i, j;
    buffer_size = 0;
    while (1) {
        read_size = read_(STDIN_FILENO, buf + buffer_size, BUFFER_CAPACITY - buffer_size);
        if (read_size == -1) {
            return 1;
        }
        buffer_size += read_size;
        if (read_size == 0) {
            if (buffer_size == 0) {
                return 0;
            }
            if (write_(STDOUT_FILENO, buf, buffer_size) == -1) {
                return 1;
            }
            return 0;
        }
        for (i = 0; i < buffer_size; i++) {
            j = 0;
            while (i + j < buffer_size && j < delstr_size && args[1][j] == buf[i + j]) {
                j++;
            }
            if (j == delstr_size) {
                for (j = i; j + delstr_size < buffer_size; j++) {
                    buf[j] = buf[j + delstr_size];
                }
                buffer_size -= delstr_size;
            }
        }
        if (i + j == buffer_size) {
            write_(STDOUT_FILENO, buf, buffer_size - j);
            for (j = 0; i + j < buffer_size; j++) {
                buf[j] = buf[i + j];
            }
            buffer_size -= i;
        }
        else {
            write_(STDOUT_FILENO, buf, buffer_size);
            buffer_size = 0;
        }
    }
}
