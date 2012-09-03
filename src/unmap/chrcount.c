/*
 * $Id: chrcount.c,v 1.8 2012/09/02 22:15:18 tom Exp $
 *
 * Title:	chrcount.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Count characters from one or more files, optionally converting
 *		on the fly from printable form to to nonprinting form.
 *		This is useful for getting metrics from map/unmap output files.
 */

#include "unmap.h"

#include <sys/types.h>
#include <sys/stat.h>

#define typeCalloc(count,type) (type *)calloc(count,sizeof(type))

typedef struct {
    char *name;
    long count;
} COUNTS;

static int m_opt;
static int p_opt;

static void
trim(char *buffer)
{
    char *s = buffer + strlen(buffer);
    while (s-- > buffer && (*s == '\r' || *s == '\n')) {
	*s = '\0';
    }
}

static char *
strmalloc(char *name)
{
    return strcpy((char *) malloc(strlen(name) + 1), name);
}

static int
isdirectory(char *path)
{
    struct stat sb;
    return (stat(path, &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFDIR);
}

static int
filesize(char *path)
{
    struct stat sb;
    return ((stat(path, &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFREG)
	    ? (int) sb.st_size
	    : -1);
}

static long
do_count(char *path)
{
    long result = filesize(path);

    if (m_opt && result >= 0) {
	FILE *fp = fopen(path, "r");
	if (fp != 0) {
	    result = map(fp, (FILE *) 0, 0);
	    fclose(fp);
	}
    }
    return result;
}

static COUNTS *
chrcount(char *path)
{
    char temp[1024];
    char leaf[1024];
    size_t used = 0;
    size_t need = 2;
    COUNTS *result = typeCalloc(need, COUNTS);

    if (isdirectory(path)) {
	FILE *pp = popen("ls -1 -a", "r");
	if (pp != 0) {
	    while (fgets(leaf, sizeof(leaf), pp) != 0) {
		trim(leaf);
		sprintf(temp, "%s/%s", path, leaf);
		if ((used + 1) >= need) {
		    need = (used + 1) * 2;
		    result = realloc(result, sizeof(COUNTS) * need);
		}
		if ((result[used].count = do_count(temp)) >= 0) {
		    result[used].name = strmalloc(leaf);
		    used++;
		    result[used].name = 0;
		}
	    }
	    pclose(pp);
	}
    } else {
	result[0].name = strmalloc(path);
	result[0].count = do_count(path);
    }
    return result;
}

static long
find_count(char *name, COUNTS * list)
{
    int n;
    for (n = 0; list[n].name != 0; n++)
	if (!strcmp(name, list[n].name))
	    return list[n].count;
    return -1;
}

static void
usage(void)
{
    fprintf(stderr, "Usage: chrcount [-m] [-p] files_or_directories\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    int n, m;
    COUNTS **vector = typeCalloc((size_t) argc, COUNTS *);
    int *widths = typeCalloc((size_t) (argc + 1), int);
    char **labels;
    int lwidth;

    while ((n = getopt(argc, argv, "mp")) != EOF) {
	switch (n) {
	case 'm':
	    m_opt = 1;
	    break;
	case 'p':
	    p_opt = 1;
	    break;
	default:
	    usage();
	}
    }
    if (optind >= argc)
	usage();

    /* count everything */
    labels = argv + optind;
    for (n = optind, m = 0; n < argc; n++) {
	widths[m] = (int) strlen(argv[n]) + 1;
	if (widths[m] < 8)
	    widths[m] = 8;
	vector[m++] = chrcount(argv[n]);
    }
    /* FIXME: should expand the first column to allow for names that appear
     * in other columns, but not in the first
     */

    /* compute the label-width */
    for (n = 0, lwidth = 8; vector[0][n].name != 0; n++) {
	if (lwidth < (int) strlen(vector[0][n].name) + 1)
	    lwidth = (int) strlen(vector[0][n].name) + 1;
    }

    /* write the header */
    printf("%-*.*s", lwidth, lwidth, "filename");
    for (n = 0; labels[n] != 0; n++)
	printf("%*s", widths[n], labels[n]);
    printf("\n");

    /* write the rows */
    for (n = 0; vector[0][n].name != 0; n++) {
	printf("%-*.*s", lwidth, lwidth, vector[0][n].name);
	for (m = 0; labels[m] != 0; m++) {
	    long value = find_count(vector[0][n].name, vector[m]);
	    if (value >= 0) {
		if (p_opt && vector[0][n].count) {
		    printf("%*.1f%%", widths[m] - 1,
			   (100.0 * (double) value) / (double) vector[0][n].count);
		} else {
		    printf("%*ld", widths[m], value);
		}
	    } else
		printf("%*s", widths[m], "n/a");
	}
	printf("\n");
    }
    free(vector);
    free(widths);
    return EXIT_SUCCESS;
}
