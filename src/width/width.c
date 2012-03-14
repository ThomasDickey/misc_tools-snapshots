/*
 * $Id: width.c,v 1.10 2012/03/13 21:06:21 tom Exp $
 *
 * Title:	width.c
 * Author:	T.Dickey
 * Created:	21 Nov 1996 (from earlier version)
 * Function:	Display lines from a text file that are longer than a given
 *		threshold (usually 80).
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

static int opt_width = -1;
static int opt_tabs = -1;
static int opt_sums = 0;
static int opt_show = 1;
static int opt_number = 0;
static int max_width = 0;
static int per_file = 0;

static int *vec_sums;
static size_t len_sums;
static int max_sums;
static long wide_lines;
static long total_lines;

static void
failed(char *s)
{
    perror(s);
    exit(1);
}

/* report lines that are wider than the threshold */
static void
report_width(char *name, int lineno, int column, char *buffer)
{
    int doit = 0;

    if (opt_width > 0) {
	if (column > opt_width) {
	    doit = opt_show;
	    wide_lines++;
	}
    }
    if (column > max_width)
	max_width = column;

    if (doit) {
	if (opt_number)
	    printf("%s:%d:", name, lineno);
	printf("%d:%s", column, buffer);
    }

    if (opt_sums) {
	if ((size_t) (column + 1) >= len_sums) {
	    size_t new_sums = 2 * (column + 1);
	    vec_sums = realloc(vec_sums, new_sums * sizeof(int));
	    while (len_sums < new_sums)
		vec_sums[len_sums++] = 0;
	}
	if (column > max_sums)
	    max_sums = column;
	vec_sums[column] += 1;
	total_lines++;
    }
}

static void
width(char *name, FILE *fp)
{
    int lineno = 1;
    int column = 0;
    int length = 0;
    int c;
    size_t have = BUFSIZ;
    char *buffer = malloc(have);

    while ((c = fgetc(fp)) != EOF) {
	if ((size_t) (length + 2) > have)
	    buffer = realloc(buffer, have *= 2);
	buffer[length++] = c;
	buffer[length] = '\0';

	/* compute the effective line & column */
	if (c == '\n') {
	    report_width(name, lineno++, column, buffer);
	    column = 0;
	    length = 0;
	} else if (opt_tabs > 0 && c == '\t') {
	    column = opt_tabs * ((column / opt_tabs) + 1);
	} else if (isprint(c)) {
	    column++;
	} else if (opt_tabs <= 0) {
	    column++;
	}
    }

    if (length > 0)
	report_width(name, lineno++, column, buffer);
    if (per_file) {
	printf("%6d\t%s\n", max_width, name);
	max_width = 0;
    }
    free(buffer);
}

static void
usage(void)
{
    static const char *tbl[] =
    {
	"Usage: width [options] [files]"
	,""
	,"Options:"
	,"  -4     set tabs to 4"
	,"  -8     set tabs to 8"
	,"  -n     show line-numbers of wide lines"
	,"  -p     report per-file (otherwise the maximum of all files is computed)"
	,"  -q     suppress listing of lines wider than the -w option"
	,"  -s     print summary showing the number of lines for each length"
	,"  -t XX  set tabs to XX"
	,"  -w XX  set threshold to XX, showing all lines that are wider"
	,""
	,"(If you do not specify tabs, they will be counted as single-columns)"
	,""
	,"Use a '-' instead of [files] to process a list of filenames from the"
	,"standard input."
    };
    int n;
    for (n = 0; n < (int) (sizeof(tbl) / sizeof(tbl[0])); n++)
	fprintf(stderr, "%s\n", tbl[n]);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int c;

    while ((c = getopt(argc, argv, "pw:nqst:48")) != EOF) {
	switch (c) {
	case 'w':
	    opt_width = atoi(optarg);
	    break;
	case 'n':
	    opt_number = 1;
	    break;
	case 'p':
	    per_file = 1;
	    break;
	case '4':
	    opt_tabs = 4;
	    break;
	case '8':
	    opt_tabs = 8;
	    break;
	case 'q':
	    opt_show = 0;
	    break;
	case 's':
	    opt_sums = 1;
	    break;
	case 't':
	    opt_tabs = atoi(optarg);
	    break;
	default:
	    usage();
	}
    }

    if (opt_sums) {
	vec_sums = (int *) calloc(len_sums = 256, sizeof(int *));
    }

    if (optind < argc) {
	while (optind < argc) {
	    char *name = argv[optind++];
	    if (!strcmp(name, "-")) {
		char buffer[BUFSIZ];
		while (fgets(buffer, sizeof(buffer), stdin) != 0) {
		    FILE *fp = fopen(buffer, "r");
		    if (fp == 0)
			failed(buffer);
		    width(buffer, fp);
		    (void) fclose(fp);
		}
	    } else {
		FILE *fp = fopen(name, "r");
		if (fp == 0)
		    failed(name);
		width(name, fp);
		(void) fclose(fp);
	    }
	}
    } else {
	width("<stdin>", stdin);
    }

    if (!per_file)
	printf("%d\n", max_width);

    if (opt_sums) {
	long this_lines = 0;
	printf("Summary:\n");
	printf("%6ld lines\n", total_lines);
	printf("%6ld wide lines\n", wide_lines);
	printf("\n");
	printf("Width  Count  Cumulative\n");
	while (max_sums >= 0) {
	    if ((c = vec_sums[max_sums]) != 0) {
		this_lines += c;
		printf("%6d:%6d %5.1f%%%s\n",
		       max_sums, c,
		       (this_lines * 100.0) / total_lines,
		       max_sums == opt_width ? " <--" : "");
	    }
	    max_sums--;
	}
    }

    return 0;
}
