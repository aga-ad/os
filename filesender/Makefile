all: filesender

filesender: filesender.c
	gcc -Wall -I../lib -L../lib -o filesender filesender.c -lbufio

clean:
	rm -f ./filesender
