all: lib/libhelpers.so cat/cat revwords/revwords

lib/libhelpers.so: lib/Makefile
	make -C lib

cat/cat: cat/Makefile
	make -C cat

revwords/revwords: revwords/Makefile
	make -C revwords

clean:
	make clean -C lib
	make clean -C cat
	make clean -C revwords
