/*
 * $Id: timerge.c,v 1.6 2020/12/19 12:14:42 tom Exp $
 *
 * Title:	timerge.c - merge a split terminfo.src
 * Author:	T.E.Dickey
 * Created:	16 May 1998
 * Function:	Merges parts for a split-up terminfo source file (usually
 *		terminfo.src) from a parent file which was processed by
 *		tisplit.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <td_getline.h>

#define is_comment(s) ((*s) == '#')

static const char *target = "TiSplit";

static void
failed(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void
append(const char *name, FILE *ofp)
{
    size_t have = BUFSIZ;
    char *temp = malloc(have);
    char *t;
    FILE *ifp;

    sprintf(temp, "%s/", target);
    t = temp + strlen(temp);
    while (!isspace((unsigned char) *name))
	*t++ = *name++;
    *t = 0;

    if ((ifp = fopen(temp, "r")) == 0)
	failed(temp);

    while (getline(&temp, &have, ifp) >= 0)
	fputs(temp, ofp);
    fclose(ifp);
    free(temp);
}

static void
timerge(const char *path)
{
    FILE *hdr;
    FILE *ofp;
    char *name = malloc(strlen(path) + 4);
    char *temp = malloc(strlen(target) + 5);
    char *bfr = 0;
    size_t have = 0;

    sprintf(name, "%s.in", path);
    if ((hdr = fopen(name, "r")) == 0)
	failed(name);
    free(name);

    sprintf(temp, "%s.out", target);
    remove(temp);
    if ((ofp = fopen(temp, "w")) == 0)
	failed(temp);

    while (getline(&bfr, &have, hdr) >= 0) {
	if (is_comment(bfr)) {
	    fputs(bfr, ofp);
	} else {
	    append(bfr, ofp);
	}
    }
    fclose(hdr);
    fclose(ofp);
    free(bfr);

    rename(temp, path);
    free(temp);
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
