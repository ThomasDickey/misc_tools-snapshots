/*
 * $Id: count_files.c,v 1.2 2012/09/02 22:17:06 tom Exp $
 *
 * Title:	count_files.c
 * Author:	T.E.Dickey
 * Created:	28 Jun 1994
 * Modified:
 *
 * Function:	Like the UNIX utility 'wc', except that this assumes that
 *		the standard input is a list of pathnames.
 *
 * Todo:	make this count binary stuff just like 'wc'.
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef	TRUE
#define TRUE 1
#define FALSE 0
#endif

/******************************************************************************/

#define	EOS	'\0'

static int l_opt;
static int w_opt;
static int c_opt;

static long total_lines;
static long total_words;
static long total_chars;

/******************************************************************************/
static void
Show(const char *path,
     long lines,
     long words,
     long chars)
{
    if (l_opt)
	(void) printf("%8ld", lines);
    if (w_opt)
	(void) printf("%8ld", words);
    if (c_opt)
	(void) printf("%8ld", chars);

    (void) printf(" %s\n", path);

    total_lines += lines;
    total_words += words;
    total_chars += chars;
}

static void
DoPath(char *path)
{
    long lines = 0;
    long words = 0;
    long chars = 0;
    FILE *fp;
    struct stat sb;

    if (stat(path, &sb) >= 0 && (sb.st_mode & S_IFMT) == S_IFREG) {
	if ((fp = fopen(path, "r")) != 0) {
	    int c;
	    int in_word = FALSE;
	    while (!feof(fp) && !ferror(fp)) {
		c = fgetc(fp);
		if (c == EOF)
		    break;
		chars++;
		if (c == '\n')
		    lines++;
		if (isascii(c)) {
		    if (isspace(c)) {
			if (in_word) {
			    in_word = FALSE;
			    words++;
			}
		    } else {
			in_word = TRUE;
		    }
		} else {
		    in_word = FALSE;
		}
	    }
	    if (in_word)
		words++;
	    (void) fclose(fp);
	}
    }
    Show(path, lines, words, chars);
}

static void
usage(void)
{
    static const char *tbl[] =
    {
	"Usage: count_files [-lwc] [filenames]",
	"",
	"Reads a list of pathnames from standard input (or as arguments) and",
	"count the corresponding files like 'wc'",
	"",
	"Options:",
	"  -l        count lines",
	"  -w        count words",
	"  -c        count characters",
    };
    size_t j;
    for (j = 0; j < sizeof(tbl) / sizeof(tbl[0]); j++)
	(void) fprintf(stderr, "%s\n", tbl[j]);
    exit(EXIT_FAILURE);
}

int
main(int argc,
     char *argv[])
{
    char buffer[BUFSIZ];
    int j;

    while ((j = getopt(argc, argv, "lwc")) != EOF) {
	switch (j) {
	case 'l':
	    l_opt = TRUE;
	    break;
	case 'w':
	    w_opt = TRUE;
	    break;
	case 'c':
	    c_opt = TRUE;
	    break;
	default:
	    usage();
	}
    }

    if (!(l_opt || w_opt || c_opt))
	l_opt =
	    w_opt =
	    c_opt = TRUE;

    /* If we're given explicit arguments, process them.  Otherwise, read
     * pathnames from standard input.
     */
    if (optind < argc) {
	while (optind < argc)
	    DoPath(argv[optind++]);
    } else {
	while (fgets(buffer, sizeof(buffer), stdin) != 0) {
	    size_t len = strlen(buffer);
	    while (len != 0 && isspace(buffer[len - 1]))
		buffer[--len] = EOS;
	    DoPath(buffer);
	}
    }
    Show("total", total_lines, total_words, total_chars);
    exit(EXIT_SUCCESS);
    /*NOTREACHED */
    return (EXIT_SUCCESS);
}
