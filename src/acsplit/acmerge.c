static const char Id[] = "$Id: acmerge.c,v 1.9 2002/12/30 15:01:35 tom Exp $";

/*
 * Title:	acmerge.c - merge a split aclocal.m4
 * Author:	T.E.Dickey
 * Created:	28 Aug 1997
 * Function:	Merges parts for a split-up autoconf macro file (usually
 *		aclocal.m4) from a parent file which was processed by acsplit,
 *		and the macros in a subdirectory.
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define isname(c) ((isalnum(c) || (c) == '_'))

#define VERBOSE(n) if(verbose > n) printf

static const char *target = "AcSplit";

static int do_dash = 1;
static int verbose = 0;

static void
failed(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static char *
skip_blanks(char *s)
{
    while (isspace(*s))
	s++;
    return s;
}

static int
is_comment(char *s)
{
    if (!strncmp(s, "dnl", 3)) {
	s += 3;
	if (isspace(*s) || ispunct(*s))
	    return 1;
    }
    return 0;
}

static char *
skip_comment(char *s)
{
    if (!strncmp(s, "dnl", 3)) {
	s += 3;
	if (isspace(*s))
	    return skip_blanks(s);
    }
    return 0;
}

static int
is_dashes(char *line)
{
    line = skip_blanks(line);
    line = skip_comment(line);
    if (line != 0 && strlen(line) >= 8) {
	return (!strncmp(line, "--------", 8));
    }
    return 0;
}

static void
dashes(FILE * ofp)
{
    if (do_dash) {
	int n;

	fputs("dnl ", ofp);
	for (n = 0; n < 75; n++)
	    fputc('-', ofp);
	fputc('\n', ofp);
    }
}

static void
append(char *name, FILE * ofp)
{
    char temp[BUFSIZ];
    char *t;
    FILE *ifp;
    int found = 0;
    int count = 0;
    char *next = skip_blanks(name);

    sprintf(temp, "%s/", target);
    t = temp + strlen(temp);
    while (isname(*next)) {
	*t++ = *next++;
	found = 1;
    }
    *t = 0;

    VERBOSE(0) ("appending %s\n", temp);
    if ((ifp = fopen(temp, "r")) == 0) {
	if (!strncmp(name, "AC_", 3) || !strncmp(name, "AM_", 3))
	    return;
	failed(temp);
    }

    if (found)
	dashes(ofp);
    else
	fputc('\n', ofp);
    while (fgets(temp, sizeof(temp), ifp) != 0) {
	fputs(temp, ofp);
	++count;
    }
    VERBOSE(0) ("...appended %d lines\n", count);
    fclose(ifp);
}

static void
acmerge(char *path)
{
    FILE *hdr;
    FILE *ofp;
    char name[BUFSIZ];
    char temp[BUFSIZ];
    char bfr[BUFSIZ];

    sprintf(name, "%s.in", path);
    if ((hdr = fopen(name, "r")) == 0)
	failed(name);

    sprintf(temp, "%s.out", target);
    remove(temp);
    if ((ofp = fopen(temp, "w")) == 0)
	failed(temp);

    while (fgets(bfr, sizeof(bfr), hdr) != 0) {
	VERBOSE(0) ("read %s", bfr);
	if (is_dashes(bfr)) {
	    dashes(ofp);
	} else if (is_comment(bfr)) {
	    fputs(bfr, ofp);
	} else {
	    append(bfr, ofp);
	}
    }
    fclose(hdr);
    fclose(ofp);
    rename(temp, path);
}

static void
usage(void)
{
    fprintf(stderr, "Usage: acmerge [-n] [-v] [aclocal.m4]\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int n;
    int found = 0;

    if (argc > 1) {
	for (n = 1; n < argc; n++) {
	    char *parm = argv[n];
	    if (*parm == '-') {
		while (*++parm) {
		    switch (*parm) {
		    case 'n':
			do_dash = 0;
			break;
		    case 'v':
			++verbose;
			break;
		    default:
			usage();
			break;
		    }
		}
	    } else {
		acmerge(parm);
		found++;
	    }
	}
    }
    if (!found) {
	acmerge("aclocal.m4");
    }

    return EXIT_SUCCESS;
}
