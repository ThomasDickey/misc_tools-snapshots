/* $Id: unmap.h,v 1.5 2004/12/31 20:36:07 tom Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>

#define ESCAPE '\033'
#define BACKSLASH '\\'

extern int unmap(FILE *ifp, FILE *ofp, int utf8);
extern int map(FILE *ifp, FILE *ofp, int utf8);
