all: filter

filter: filter.c
	gcc -Wall -I../lib -L../lib -o filter filter.c -lhelpers

clean:
	rm -f ./filter
