static const char Id[] = "$Id: timerge.c,v 1.2 2000/03/05 03:03:48 tom Exp $";

/*
 * Title:	timerge.c - merge a split aclocal.m4
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

#define is_comment(s) ((*s) == '#')

static const char *target = "TiSplit";

static void failed(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static void append(char *name, FILE *ofp)
{
	char temp[BUFSIZ];
	char *t;
	FILE *ifp;

	sprintf(temp, "%s/", target);
	t = temp + strlen(temp);
	while (!isspace(*name))
		*t++ = *name++;
	*t = 0;

	if ((ifp = fopen(temp, "r")) == 0)
		failed(temp);

	while (fgets(temp, sizeof(temp), ifp) != 0)
		fputs(temp, ofp);
	fclose(ifp);
}

static void timerge(char *path)
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
		if (is_comment(bfr)) {
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
			timerge(argv[n]);
	} else {
		timerge("terminfo.src");
	}

	return EXIT_SUCCESS;
}
