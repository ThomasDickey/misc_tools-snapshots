# $Id: Makefile,v 1.2 1997/06/09 23:48:27 tom Exp $

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

check: map unmap
	sh -c ./run_test.sh
