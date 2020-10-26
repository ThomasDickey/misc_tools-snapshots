/* $Id: unmap.h,v 1.6 2020/10/25 18:03:36 tom Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define ESCAPE '\033'
#define BACKSLASH '\\'

extern int unmap(FILE *ifp, FILE *ofp, int utf8);
extern int map(FILE *ifp, FILE *ofp, int utf8);
