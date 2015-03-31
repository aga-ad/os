#include <bufio.h>

int main() {
    struct buf_t *buf= buf_new(4096);
    ssize_t read_size;
    while (1) {
        read_size = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
        if (read_size == -1) {
            return 1;
        }
        if (read_size == 0) {
            return 0;
        }
        if (buf_flush(STDOUT_FILENO, buf, read_size) == -1) {
            return 1;
        }
    }
}
