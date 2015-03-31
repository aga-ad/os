all: bufcat

bufcat: bufcat.c
	gcc -Wall -I../lib -L../lib -o bufcat bufcat.c -lbufio

clean:
	rm -f ./bufcat
