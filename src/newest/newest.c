/*
 * $Id: newest.c,v 1.3 2012/09/02 22:10:29 tom Exp $
 *
 * Given one or more files specified either on the command line, or via a pipe,
 * compute the date and/or name of the newest one, printing to stdout.
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int found = 0;
static time_t the_date;
static char the_name[BUFSIZ];

static void
newest(char *name)
{
    struct stat sb;
    if (lstat(name, &sb) >= 0
	&& (sb.st_mode & S_IFMT) == S_IFREG) {
	if (!found || sb.st_mtime > the_date) {
	    the_date = sb.st_mtime;
	    strcpy(the_name, name);
	    found = 1;
	}
    }
}

int
main(int argc, char *argv[])
{
    int n;
    int d_opt = 0;
    int n_opt = 0;

    while ((n = getopt(argc, argv, "dn")) != EOF) {
	switch (n) {
	case 'd':
	    d_opt = 1;
	    break;
	case 'n':
	    n_opt = 1;
	    break;
	}
    }
    if (!n_opt && !d_opt)
	d_opt = 1;
    if (optind < argc) {
	while (optind < argc)
	    newest(argv[optind++]);
    } else {
	char buffer[BUFSIZ];
	while (fgets(buffer, sizeof(buffer), stdin)) {
	    size_t nn = strlen(buffer);
	    if (nn != 0 && buffer[--nn] == '\n')
		buffer[nn] = 0;
	    newest(buffer);
	}
    }
    if (found) {
	if (n_opt) {
	    printf("%s%c", the_name, d_opt ? '\t' : '\n');
	}
	if (d_opt) {
	    printf("%s", ctime(&the_date));
	}
    }
    return 0;
}
