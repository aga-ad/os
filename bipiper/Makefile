all: forking polling

forking: forking.c
	gcc -Wall -I../lib -L../lib -o forking forking.c -lbufio

polling: polling.c
	gcc -Wall -I../lib -L../lib -o polling polling.c -lbufio

clean:
	rm -f ./forking
	rm -f ./polling
