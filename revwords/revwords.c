#include <helpers.h>
#include <stdio.h>

void reverse(char* buf, size_t size) {
	size_t i;
	char t;
	for (i = 0; i < size / 2; i++) {
		t = buf[size - 1 - i];
		buf[size - 1 - i] = buf[i];
		buf[i] = t;
	}
}

int main() {
    const size_t BUFFER_CAPACITY = 4096;
    const size_t MAX_WORD_LENGTH = 4096;//"Гарантируется, что каждое слово имеет длину не более 4096 байт."
	char buf[BUFFER_CAPACITY + 1];
	int i, l;
	ssize_t read_size, write_size, buffer_size;
	buffer_size = 0;
	while (1) {
		read_size = read_until(STDIN_FILENO, buf + buffer_size, BUFFER_CAPACITY - buffer_size, ' ');
		if (read_size == -1) {
			return 1;
		}
		if (read_size == 0) {
			if (buffer_size == 0) {
				return 0;
			}
			reverse(buf, buffer_size);
			write_size = write_(STDOUT_FILENO, buf, buffer_size);
			if (write_size == -1) {
				return 1;
			}
			return 0;
		}
		i = buffer_size;
		buffer_size += read_size;
		l = 0;
		for (; i < buffer_size; i++) {
			if (buf[i] == ' ') {
				reverse(buf + l, i - l);
				write_size = write_(STDOUT_FILENO, buf + l, i - l + 1);
				if (write_size == -1) {
					return 1;
				}
				l = i + 1;
			}
		}
		if (buffer_size - l == MAX_WORD_LENGTH) {
			reverse(buf, buffer_size);
			write_size = write_(STDOUT_FILENO, buf, buffer_size);
			if (write_size == -1) {
				return 1;
			}
			l = buffer_size;
		}
		buffer_size -= l;
		for (i = 0; i < buffer_size; i++) {
			buf[i] = buf[i + l];
		}
	}
}
