/*
 * Title:	chrcount.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Count characters from one or more files, optionally converting
 *		on the fly from printable form to to nonprinting form.
 */

static const char Id[] = "$Id: chrcount.c,v 1.4 2004/12/31 20:36:18 tom Exp $";

#include "unmap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define typeCalloc(count,type) (type *)calloc(count,sizeof(type))

typedef struct {
    char *name;
    long count;
} COUNTS;

static int m_opt;
static int p_opt;

static char *
strmalloc(char *name)
{
    return strcpy(malloc(strlen(name)) + 1, name);
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
    return (stat(path, &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFREG) ?
	sb.st_size : -1;
}

static int
do_select(const struct dirent *de)
{
    /* select everything, so we don't do 'stat()' twice */
    return 1;
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
    char temp[PATH_MAX];
    size_t used = 0;
    size_t need = 0;
    COUNTS *result = typeCalloc(2, COUNTS);

    if (isdirectory(path)) {
	int n;
	struct dirent **namelist;
	n = scandir(path, &namelist, do_select, alphasort);
	if (n > 0) {
	    need = n;
	    result = typeCalloc(need + 1, COUNTS);
	    for (n = 0, used = 0; n < need; n++) {
		char *leaf = namelist[n]->d_name;
		sprintf(temp, "%s/%s", path, leaf);
		if ((result[used].count = do_count(temp)) >= 0) {
		    result[used].name = leaf;
		    used++;
		}
	    }
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
	widths[m] = strlen(argv[n]) + 1;
	if (widths[m] < 8)
	    widths[m] = 8;
	vector[m++] = chrcount(argv[n]);
    }
    /* FIXME: should expand the first column to allow for names that appear
     * in other columns, but not in the first
     */

    /* compute the label-width */
    for (n = 0, lwidth = 8; vector[0][n].name != 0; n++) {
	if (lwidth < strlen(vector[0][n].name) + 1)
	    lwidth = strlen(vector[0][n].name) + 1;
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
		    printf("%*.1f%%", widths[m] - 1, (100.0) * value /
			   vector[0][n].count);
		} else {
		    printf("%*ld", widths[m], value);
		}
	    } else
		printf("%*s", widths[m], "n/a");
	}
	printf("\n");
    }
    return EXIT_SUCCESS;
}
