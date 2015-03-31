all: lib/libhelpers.so cat/cat revwords/revwords filter/filter bufcat/bufcat

lib/libhelpers.so: lib/Makefile
	make -C lib

cat/cat: cat/Makefile
	make -C cat

revwords/revwords: revwords/Makefile
	make -C revwords

filter/filter: filter/Makefile
	make -C filter

bufcat/bufcat: bufcat/Makefile
	make -C bufcat

clean:
	make clean -C lib
	make clean -C cat
	make clean -C revwords
	make clean -C filter
	make clean -C bufcat
