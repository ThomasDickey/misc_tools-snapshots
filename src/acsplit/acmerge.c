static const char Id[] = "$Id: acmerge.c,v 1.3 1997/09/06 23:08:51 tom Exp $";

/*
 * Title:	acmerge.c - merge a split aclocal.m4
 * Author:	T.E.Dickey
 * Created:	28 Aug 1997
 * Function:	Merges parts for a split-up autoconf macro file (usually
 *		aclocal.m4) from a parent file which was processed by acsplit,
 *		and the macros in a subdirectory.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define isname(c) ((isalnum(c) || (c) == '_'))

static const char *target = "AcSplit";

static void failed(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static char *skip_blanks(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

static int is_comment(char *s)
{
	if (!strncmp(s, "dnl", 3)) {
		s += 3;
		if (isspace(*s) || ispunct(*s))
			return 1;
	}
	return 0;
}

static char *skip_comment(char *s)
{
	if (!strncmp(s, "dnl", 3)) {
		s += 3;
		if (isspace(*s))
			return skip_blanks(s);
	}
	return 0;
}

static int is_dashes(char *line)
{
	line = skip_blanks(line);
	line = skip_comment(line);
	if (line != 0) {
		return (!strncmp(line, "--------", 8));
	}
	return 0;
}

static void dashes(FILE *ofp)
{
	int n;

	fputs("dnl ", ofp);
	for (n = 0; n < 75; n++)
		fputc('-', ofp);
	fputc('\n', ofp);
}

static void append(char *name, FILE *ofp)
{
	char temp[BUFSIZ];
	char *t;
	FILE *ifp;

	name = skip_blanks(name);
	sprintf(temp, "%s/", target);
	t = temp + strlen(temp);
	while (isname(*name))
		*t++ = *name++;
	*t = 0;

	if ((ifp = fopen(temp, "r")) == 0)
		failed(temp);

	dashes(ofp);
	while (fgets(temp, sizeof(temp), ifp) != 0)
		fputs(temp, ofp);
}

static void acmerge(char *path)
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

int
main(int argc, char *argv[])
{
	int n;

	if (argc > 1) {
		for (n = 1; n < argc; n++)
			acmerge(argv[n]);
	} else {
		acmerge("aclocal.m4");
	}

	return EXIT_SUCCESS;
}
