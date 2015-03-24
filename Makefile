all: lib/libhelpers.so cat/cat revwords/revwords filter/filter

lib/libhelpers.so: lib/Makefile
	make -C lib

cat/cat: cat/Makefile
	make -C cat

revwords/revwords: revwords/Makefile
	make -C revwords

filter/filter: filter/Makefile
	make -C filter

clean:
	make clean -C lib
	make clean -C cat
	make clean -C revwords
	make clean -C filter
