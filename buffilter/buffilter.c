#include <helpers.h>
#include <bufio.h>

//argument length must be less than BUFFER_CAPACITY
int main(int argn, char** argv) {
    const size_t MAX_ARGUMENTS_NUMBER = 256;
    int status, last_argument;
    ssize_t read_size;
    char* args[MAX_ARGUMENTS_NUMBER];
    for (last_argument = 0; last_argument + 1 < argn; last_argument++) {
        args[last_argument] = argv[last_argument + 1];
    }
    struct buf_t *out = buf_new(4096);
    struct buf_t *in = buf_new(4096);
    char str[4096];
    while (1) {
        read_size = buf_getline(STDIN_FILENO, in, str);
        if (read_size == -1) {
            return 1;
        }
        if (read_size == 0) {
            buf_flush(STDOUT_FILENO, out, buf_size(out));
            return 0;
        }
        args[last_argument] = str;
        if (str[read_size - 1] == '\n') {
            read_size--;
        }
        str[read_size] = 0;
        status = spawn(args[0], args);
        if (status == 0) {
            str[read_size] = '\n';
            if (buf_write(STDOUT_FILENO, out, str, read_size + 1) == -1) {
                return 1;
            }
        }
    }
    return 0;
}
