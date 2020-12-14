/*
 * $Id: tisplit.c,v 1.3 2020/12/14 00:34:24 tom Exp $
 *
 * Title:	tisplit.c - split terminfo.src
 * Author:	T.E.Dickey
 * Created:	16 May 1998
 * Function:	Splits a terminfo file (usually terminfo.src) into
 *		a parent file which can be reprocessed by timerge, and
 *		the entries in a subdirectory.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define VERBOSE(n) if(verbose > n) printf
#define isname(c) (isprint((unsigned char)c) && (c) != '#' && !isspace((unsigned char)c) && (c) != '|')

static const char *target = "TiSplit";
static int verbose;

static void
failed(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/* parse the terminal name from the first line of the description */
static int
defined(char *line, char *name)
{
    if (isname(*line)) {
	char *s = line;
	char *t = name;
	while (isname(*s))
	    *t++ = *s++;
	*t = 0;
	return 1;
    }
    return 0;
}

static char *
my_temp(char *dst)
{
    sprintf(dst, "%s/%s.out", target, target);
    return dst;
}

static FILE *
finish(FILE *ofp, char *name)
{
    char temp[BUFSIZ];

    my_temp(temp);
    if (ofp != 0)
	fclose(ofp);

    if (*name != 0) {
	char dst[BUFSIZ];

	VERBOSE(1) ("finish %s\n", name);

	sprintf(dst, "%s/%s", target, name);
	rename(temp, dst);
	*name = 0;

	remove(temp);
    }
    if ((ofp = fopen(temp, "w")) == 0)
	failed(temp);
    return ofp;
}

static FILE *
append(FILE *ofp, FILE *hdr)
{
    char temp[BUFSIZ];
    char bfr[BUFSIZ];

    if (ofp != 0) {

	fclose(ofp);
	my_temp(temp);
	VERBOSE(1) ("append %s\n", temp);

	ofp = fopen(temp, "r");
	while (fgets(bfr, sizeof(bfr), ofp)) {
	    VERBOSE(2) ("*...%s", bfr);
	    fputs(bfr, hdr);
	}
	fclose(ofp);

	remove(temp);
	if ((ofp = fopen(temp, "w")) == 0)
	    failed(temp);
    }
    return ofp;
}

static void
tisplit(const char *path)
{
    FILE *hdr;
    FILE *ifp = fopen(path, "r");
    FILE *ofp;
    char name[BUFSIZ];
    char bfr[BUFSIZ];

    if (ifp == 0)
	failed(path);
    mkdir(target, 0777);

    sprintf(name, "%s.in", path);
    remove(name);
    if ((hdr = fopen(name, "w")) == 0)
	failed(name);

    name[0] = 0;
    ofp = finish((FILE *) 0, name);

    while (fgets(bfr, sizeof(bfr), ifp) != 0) {
	VERBOSE(2) (">...%s", bfr);
	if (*bfr == '#' || *bfr == '\n' || *bfr == '\r') {
	    if (*name != 0) {
		ofp = finish(ofp, name);
	    }
	} else if (isname(*bfr) && (*name != 0)) {
	    ofp = finish(ofp, name);
	}
	fputs(bfr, ofp);
	if (defined(bfr, name)) {
	    fprintf(hdr, "%s\n", name);
	}
    }
    if (*name != 0)
	ofp = finish(ofp, name);
    else
	ofp = append(ofp, hdr);
    fclose(hdr);
    remove(my_temp(name));
}

int
main(int argc, char *argv[])
{
    int n;
    int found = 0;

    if (argc > 1) {
	for (n = 1; n < argc; n++) {
	    if (!strcmp(argv[n], "-v"))
		verbose++;
	    else {
		tisplit(argv[n]);
		found++;
	    }
	}
    }
    if (!found)
	tisplit("terminfo.src");

    return EXIT_SUCCESS;
}
