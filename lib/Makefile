all: libhelpers.so

libhelpers.so: helpers.o
	gcc -o libhelpers.so -shared helpers.o

helpers.o: helpers.c
	gcc -o helpers.o -c -fPIC -Wall helpers.c

clean:
	rm -f libhelpers.so
	rm -f helpers.o
