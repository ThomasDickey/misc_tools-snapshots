static const char Id[] = "$Id: acsplit.c,v 1.5 1997/09/06 23:07:58 tom Exp $";

/*
 * Title:	acsplit.c - split aclocal.m4
 * Author:	T.E.Dickey
 * Created:	28 Aug 1997
 * Function:	Splits an autoconf macro file (usually aclocal.m4) into
 *		a parent file which can be reprocessed by acmerge, and
 *		the macros in a subdirectory.
 *
 * FIXME:	The splitting is crude, not taking into account ()/[] balancing.
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
#define isname(c) ((isalnum(c) || (c) == '_'))

static const char *target = "AcSplit";
static int verbose;

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
	while (*s != '\0' && strchr("[()]", *s))
		s++;
	return s;
}

static char *skip_comment(char *s)
{
	if (!strncmp(s, "dnl", 3)) {
		s += 3;
		if (isspace(*s) || ispunct(*s))
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
	if (((s = match(line, "AC_DEFUN")) != 0)
	 || ((s = match(line, "define")) != 0)) {
		char temp[BUFSIZ];
		char *t = temp;

		s = skip_blanks(s);
		s = skip_punct(s);
		s = skip_blanks(s);
		while (isname(*s))
			*t++ = *s++;
		*t = 0;
		if (t != temp) {
			strcpy(name, temp);
			return 1;
		}
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

static FILE *append(FILE *ofp, FILE *hdr)
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
		if (is_dashes(bfr)) {
			VERBOSE(2) (">%ld (%s) %p\n", ftell(ifp), name, ofp);
			if (*name != 0) {
				ofp = finish(ofp, name);
			} else {
				ofp = append(ofp, hdr);
				if (content)
					fputs(bfr, hdr);
			}
			content = 0;
		} else {
			VERBOSE(2) (">...%s", bfr);
			fputs(bfr, ofp);
			content++;
		}
		if (defined(bfr, name)) {
			fprintf(hdr, "%s\n", name);
		}

#if 0
		if (*name == 0 && !content)
			fputs(bfr, hdr);
#endif
	}
	ofp = finish(ofp, name);
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
				acsplit(argv[n]);
				found++;
			}
		}
	}
	if (!found)
		acsplit("aclocal.m4");

	return EXIT_SUCCESS;
}
