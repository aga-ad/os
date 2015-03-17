#include <helpers.h>
#include <stdio.h>

int main() {
    const size_t BUFFER_SIZE = 4096;
	char buf[BUFFER_SIZE];
	int i;
	int lim;
	while (1) {
		ssize_t read_size = read_until(STDIN_FILENO, buf, BUFFER_SIZE, ' ');
		if (read_size == -1) {
			return 1;
		}
		if (read_size == 0) {
			return 0;
		}
		if (buf[read_size - 1] == ' ') {
			lim = read_size - 1;
		}
		else {
			lim = read_size;
		}
		for (i = 0; i < lim / 2; i++) {
			buf[i] ^= buf[lim - 1 - i];
			buf[lim - 1 - i] ^= buf[i];
			buf[i] ^= buf[lim - 1 - i];
		}
		ssize_t write_size = write_(STDOUT_FILENO, buf, read_size);
		if (write_size == -1) {
			return 1;
		}
	}
}
