/*
 * $Id: map.c,v 1.6 2020/10/25 18:04:06 tom Exp $
 *
 * Title:	map.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Translate one or more files containing printing characters
 *		into nonprinting form.
 */

#include "unmap.h"

#include <td_getopt.h>

static void
usage(void)
{
    fprintf(stderr, "Usage: map [-u] [files]\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    int ch;
    int utf8 = 0;

    while ((ch = getopt(argc, argv, "u")) != EOF) {
	switch (ch) {
	case 'u':
	    utf8 = 1;
	    break;
	default:
	    usage();
	    break;
	}
    }

    if (argc > optind) {
	int n;
	for (n = 1; n < argc; n++) {
	    FILE *fp = fopen(argv[n], "r");
	    if (fp != 0) {
		map(fp, stdout, utf8);
		fclose(fp);
	    }
	}
    } else {
	map(stdin, stdout, utf8);
    }
    return 0;
}
