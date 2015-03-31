all: libhelpers.so libbufio.so

libhelpers.so: helpers.o
	gcc -o libhelpers.so -shared helpers.o

helpers.o: helpers.c
	gcc -o helpers.o -c -fPIC -Wall helpers.c

libbufio.so: bufio.o
	gcc -o libbufio.so -shared bufio.o

bufio.o: bufio.c
	gcc -o bufio.o -c -fPIC -Wall bufio.c

clean:
	rm -f libhelpers.so
	rm -f helpers.o
	rm -f libbufio.so
	rm -f bufio.o
