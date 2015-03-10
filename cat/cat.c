#include <helpers.h>
#include <stdio.h>

int main() {
    const size_t BUFFER_SIZE = 4096;
	char buf[BUFFER_SIZE];
	while (1) {
		ssize_t read_size = read_(STDIN_FILENO, buf, BUFFER_SIZE);
		if (read_size == -1) {
			return 1;
		}
		if (read_size == 0) {
			return 0;
		}
		ssize_t write_size = write_(STDOUT_FILENO, buf, read_size);
		if (write_size == -1) {
			return 1;
		}
	}
}