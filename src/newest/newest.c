/*
 * $Id: newest.c,v 1.5 2020/12/19 11:36:11 tom Exp $
 *
 * Title:	newest.c
 * Author:	T.Dickey
 * Created:	26 Mar 2000
 * Function:	Given one or more files specified either on the command line,
 *		or via a pipe, compute the date and/or name of the newest one,
 *		printing to stdout.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <td_getline.h>
#include <td_getopt.h>

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

static void
usage(void)
{
    fprintf(stderr, "usage: newest [-d] [-n] file1 [ file2 ...]\n");
    exit(EXIT_FAILURE);
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
	default:
	    usage();
	}
    }
    if (!n_opt && !d_opt)
	d_opt = 1;
    if (optind < argc) {
	while (optind < argc)
	    newest(argv[optind++]);
    } else {
	char *buffer = 0;
	size_t have = 0;
	while (getline(&buffer, &have, stdin) >= 0) {
	    size_t nn = strlen(buffer);
	    if (nn != 0 && buffer[--nn] == '\n')
		buffer[nn] = 0;
	    newest(buffer);
	}
	free(buffer);
    }
    if (found) {
	if (n_opt) {
	    printf("%s%c", the_name, d_opt ? '\t' : '\n');
	}
	if (d_opt) {
	    printf("%s", ctime(&the_date));
	}
    }
    return EXIT_SUCCESS;
}
