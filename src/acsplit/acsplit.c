static const char Id[] = "$Id: acsplit.c,v 1.1 1997/08/28 23:25:34 tom Exp $";

/*
 * Title:	acsplit.c - split aclocal.m4
 * Author:	T.E.Dickey
 * Created:	28 Aug 1997
 * Function:	Splits an autoconf macro file (usually aclocal.m4) into
 *		a parent file which can be reprocessed by acmerge, and
 *		the macros in a subdirectory.
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

static char *skip_punct(char *s)
{
	while (ispunct(*s) && !isname(*s))
		s++;
	return s;
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

static int dashes(char *line)
{
	line = skip_blanks(line);
	line = skip_comment(line);
	if (line != 0) {
		return (!strncmp(line, "--------", 8));
	}
	return 0;
}

static char *match(char *s, char *name)
{
	char *base = s;

	while (*s == *name) {
		s++, name++;
	}
	return (!isname(*s) && (s != base)) ? s : 0;
}

/* AC_DEFUN([CF_ADD_INCDIR] */
static int defined(char *line, char *name)
{
	char *s;

	line = skip_blanks(line);
	if ((s = match(line, "AC_DEFUN")) != 0) {
		char *t = name;

		s = skip_blanks(s);
		s = skip_punct(s);
		s = skip_blanks(s);
		while (isname(*s))
			*t++ = *s++;
		*t = 0;
		return (t != name);
	}
	return 0;
}

static char *my_temp(char *dst)
{
	sprintf(dst, "%s/%s.out", target, target);
	return dst;
}

static FILE *finish(FILE *ofp, char *name)
{
	char temp[BUFSIZ];

	if (ofp != 0)
		fclose(ofp);

	my_temp(temp);
	if (*name != 0) {
		char dst[BUFSIZ];
		sprintf(dst, "%s/%s", target, name);
		rename(temp, dst); 	
		*name = 0;
	}

	remove(temp);
	if ((ofp = fopen(temp, "w")) == 0)
		failed(temp);
	return ofp;
}

static void acsplit(char *path)
{
	FILE *hdr;
	FILE *ifp = fopen(path, "r");
	FILE *ofp;
	char name[BUFSIZ];
	char bfr[BUFSIZ];
	int content = 0;

	if (ifp == 0)
		failed(path);
	mkdir(target, 0777);

	sprintf(name, "%s.in", path);
	remove(name);
	if ((hdr = fopen(name, "w")) == 0)
		failed(name);

	name[0] = 0;
	ofp = finish((FILE *)0, name);

	while (fgets(bfr, sizeof(bfr), ifp) != 0) {
		if (dashes(bfr)) {
			ofp = finish(ofp, name);
			content++;
		} else {
			fputs(bfr, ofp);
		}
		if (defined(bfr, name))
			fprintf(hdr, "%s\n", name);

		if (*name == 0 && !content)
			fputs(bfr, hdr);
	}
	ofp = finish(ofp, name);
	fclose(hdr);
	remove(my_temp(name));
}

int
main(int argc, char *argv[])
{
	int n;

	if (argc > 1) {
		for (n = 1; n < argc; n++)
			acsplit(argv[n]);
	} else {
		acsplit("aclocal.m4");
	}

	return EXIT_SUCCESS;
}
