/* $Id: unmap.h,v 1.2 1997/06/09 10:59:38 tom Exp $ */

#include <stdio.h>
#include <ctype.h>

#define ESCAPE '\033'
#define BACKSLASH '\\'

extern int unmap_s(FILE *ifp, FILE *ofp);
extern int map_s(FILE *ifp, FILE *ofp);
