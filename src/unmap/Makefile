# $Id: Makefile,v 1.3 1997/06/10 01:00:04 tom Exp $

CC	= gcc-normal

PROGS	= map unmap chrcount

all:	$(PROGS)

clean:
	rm -f $(PROGS) *.o

chrcount: chrcount.o map_s.o
	$(CC) -o $@ chrcount.o map_s.o

unmap: unmap.o unmap_s.o
	$(CC) -o $@ unmap.o unmap_s.o

map: map.o map_s.o
	$(CC) -o $@ map.o map_s.o

map.o map_s.o \
unmap.o \
unmap_s.o : unmap.h

check: map unmap
	sh -c ./run_test.sh
