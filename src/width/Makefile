# $Id: Makefile,v 1.1 1996/11/21 15:36:52 dickey Exp $
# makefile for 'width'

THIS	= width
CC	= gcc -Wall #-pg
CFLAGS	= #-g -DDEBUG -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion
OBJECTS	= $(THIS).o

all:	$(THIS)
clean:			; rm -f *.o *.bak *.log *.out *pure* core .pure
clobber:	clean	; rm -f $(THIS)
install:	$(THIS)
	copy -uv $(THIS) /usr/local/bin/
	copy -uv $(THIS).1 /usr/local/man/man1
lint:
	lint $(THIS).c archive.c

$(THIS):	$(OBJECTS)
	$(CC) -o $@ $(OBJECTS)
