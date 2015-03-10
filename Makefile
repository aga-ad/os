all: lib/libhelpers.so cat/cat

lib/libhelpers.so: lib/Makefile
	make -C lib

cat/cat: cat/Makefile
	make -C cat

clean:
	make clean -C lib
	make clean -C cat
