all: buffilter

buffilter: buffilter.c
	gcc -Wall -I../lib -L../lib -o buffilter buffilter.c -lbufio -lhelpers

clean:
	rm -f ./buffilter
