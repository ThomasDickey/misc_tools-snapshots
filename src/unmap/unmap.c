/*
 * Title:	unmap.c
 * Author:	T.E.Dickey
 * Created:	15 Dec 1995
 * Function:	Translate one or more files containing nonprinting characters
 *		into visible form.
 */

static const char Id[] = "$Id: unmap.c,v 1.7 2004/12/31 21:09:43 tom Exp $";

#include "unmap.h"

static void
usage(void)
{
    fprintf(stderr, "Usage: unmap [-u] [files]\n");
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
		(void) unmap(fp, stdout, utf8);
		(void) fclose(fp);
	    }
	}
    } else {
	(void) unmap(stdin, stdout, utf8);
    }
    return EXIT_SUCCESS;
}
