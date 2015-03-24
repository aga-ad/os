#include <helpers.h>

//argument length must be less than BUFFER_CAPACITY
int main(int argn, char** argv) {
    const size_t BUFFER_CAPACITY = 4096;
    char buf[BUFFER_CAPACITY];
    int i, l, status;
    ssize_t read_size, write_size, buffer_size;
    char* args[2];
    args[0] = argv[1];
    buffer_size = 0;
    while (1) {
        read_size = read_until(STDIN_FILENO, buf + buffer_size, BUFFER_CAPACITY - buffer_size, '\n');
        if (read_size == -1) {
            return 1;
        }
        if (read_size == 0) {
            if (buffer_size == 0) {
                return 0;
            }
            args[1] = buf;
            buf[buffer_size] = 0;
            status = spawn(args[0], args);
            if (status == 0) {
                buf[buffer_size] = '\n';
                write_size = write_(STDOUT_FILENO, buf, buffer_size + 1);
                if (write_size == -1) {
                    return 1;
                }
            }
            else if (status == -1) {
                return 1;
            }
            break;
        }
        i = buffer_size;
        buffer_size += read_size;
        l = 0;
        for (; i < buffer_size; i++) {
            if (buf[i] == '\n') {
                args[1] = buf + l;
                buf[i] = 0;
                status = spawn(args[0], args);
                if (status == 0) {
                    buf[i] = '\n';
                    write_size = write_(STDOUT_FILENO, buf + l, i - l + 1);
                    if (write_size == -1) {
                        return 1;
                    }
                }
                else if (status == -1) {
                    return 1;
                }
                l = i + 1;
            }
        }
        buffer_size -= l;
        for (i = 0; i < buffer_size; i++) {
            buf[i] = buf[i + l];
        }
    }
    return 0;
}
