all: lib/libhelpers.so cat/cat revwords/revwords filter/filter bufcat/bufcat simplesh/simplesh filesender/filesender bipiper/forking

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

simplesh/simplesh: simplesh/Makefile
	make -C simplesh

filesender/filesender: filesender/Makefile
	make -C filesender

bipiper/forking: bipiper/Makefile
	make -C bipiper

clean:
	make clean -C lib
	make clean -C cat
	make clean -C revwords
	make clean -C filter
	make clean -C bufcat
	make clean -C simplesh
	make clean -C filesender
	make clean -C bipiper
