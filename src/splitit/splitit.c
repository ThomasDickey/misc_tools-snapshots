/*
 * $Id: splitit.c,v 1.5 2020/12/19 12:52:13 tom Exp $
 *
 * Title:	splitit.c
 * Author:	T.E.Dickey
 * Created:	17 Oct 1994
 *
 * Function:	
 *		Reads one or more files, writing to a new set of files suffixed
 *		".001", ".002", etc.  which are all the same size.  The naming
 *		convention is chosen to match the MSDOS utility named
 *		'splitit'.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <td_getopt.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

static FILE *output;
static off_t split_size = 1024 * 1423;	/* MS-DOS 3.5" diskette */
static const char *path_prefix = "splitit";

static off_t written;
static int sequence;
static char seq_name[MAXPATHLEN];

static void
failed(const char *s)
{
    perror(s);
    if (*seq_name)
	fprintf(stderr, "...file %s\n", seq_name);
    exit(EXIT_FAILURE);
}

static void
usage(void)
{
    static const char *msg[] =
    {
	"Usage: splitit [options] [input-files]"
	,""
	,"Options:"
	,"  -p NAME  output file-specification (will append .001 etc)"
	,"  -s SIZE  output chunk-size (default 1.44M)"
    };
    size_t j;
    for (j = 0; j < sizeof(msg) / sizeof(msg[0]); j++)
	fprintf(stderr, "%s\n", msg[j]);
    exit(EXIT_FAILURE);
}

static void
open_output(void)
{
    (void) sprintf(seq_name, "%s.%03d", path_prefix, ++sequence);
    (void) unlink(seq_name);
    if ((output = fopen(seq_name, "w")) == 0)
	failed("fopen");
    written = 0;
}

static void
next_output(void)
{
    fclose(output);
    open_output();
}

static void
splitit(FILE *input)
{
    char bfr[BUFSIZ];
    int got;
    int adj;

    if (output == 0)
	open_output();

    while ((got = (int) fread(bfr, sizeof(*bfr), sizeof(bfr), input)) > 0) {
	if (got + written > split_size) {
	    adj = (int) (split_size - written);
	    got -= adj;
	    if ((int) fwrite(bfr, sizeof(*bfr), (size_t) adj, output) != adj) {
		failed("fwrite");
	    }
	    next_output();
	} else {
	    adj = 0;
	}
	if ((int) fwrite(bfr + adj, sizeof(*bfr), (size_t) got, output) != got) {
	    failed("fwrite");
	}
	written += got;
    }
    if (ferror(input))
	failed("fread");
}

int
main(int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, "s:p:")) != EOF) {
	switch (c) {
	case 's':
	    split_size = atoi(optarg);
	    break;
	case 'p':
	    path_prefix = optarg;
	    break;
	default:
	    usage();
	    /*NOTREACHED */
	}
    }
    if (optind < argc) {
	while (optind < argc) {
	    char *name = argv[optind++];
	    FILE *fp = fopen(name, "r");
	    if (fp != 0) {
		splitit(fp);
	    } else {
		failed(name);
	    }
	    (void) fclose(fp);
	}
    } else {
	splitit(stdin);
    }
    if (fclose(output) < 0)
	failed("fclose");
    exit(EXIT_FAILURE);
}
