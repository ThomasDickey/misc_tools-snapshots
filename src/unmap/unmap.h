/* $Id: unmap.h,v 1.3 1997/06/10 00:56:31 tom Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

#define ESCAPE '\033'
#define BACKSLASH '\\'

extern int unmap(FILE *ifp, FILE *ofp);
extern int map(FILE *ifp, FILE *ofp);
