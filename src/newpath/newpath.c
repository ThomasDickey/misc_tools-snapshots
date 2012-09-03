/*
 * $Id: newpath.c,v 1.14 2012/09/02 20:12:52 tom Exp $
 *
 * Author:	T.E.Dickey
 * Created:	02 Jun 1994
 * Modified:
 *		13 Mar 2012, integrate into misc_tools package
 *		10 Nov 2000, port to win32
 *
 * Function: (see usage message)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#ifdef WIN32
#define	PATHDELIM ';'
#define Compare(a,b) stricmp(a,b)
#define SameDir(a,b) (!Compare((a)->nn, (b)->nn))
#else
#define	PATHDELIM ':'
#define Compare(a,b) strcmp(a,b)
#define SameDir(a,b) ((a)->sb.st_ino == (b)->sb.st_ino \
				   && (a)->sb.st_dev == (b)->sb.st_dev)
#endif

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

#ifndef	EXIT_SUCCESS
#define	EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#define	EOS	'\0'

#define	BLANK	""

#ifdef OPT_TRACE
#define	TRACE(p)				fprintf p
#else
#define	TRACE(p)		/* fprintf p; */
#endif

typedef struct {
    char *nn;
    struct stat sb;
} LIST;

static int allow_files;

static void
failed(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void
usage(void)
{
    static const char *tbl[] =
    {
	"Usage: newpath [options [directories]] [ - command]",
	"",
	"Echos the PATH environment variable with the given directories added",
	"(the default) or removed.",
	"",
	"Options:",
	"    -a NAME modify path after given NAME",
	"    -b      put new arguments before existing path",
	"    -d      remove duplicates/non-directory items",
	"    -e      put new arguments after end of path",
	"    -f      allow filenames to match, as well as directories",
	"    -n NAME specify environment-variable to use (default: PATH)",
	"    -p      print in C-shell form",
	"    -r      remove arguments from path",
	"    -v      verbose",
	"",
	"Put a '-' before a command which is invoked with the environment variable",
	"updated, rather than echoing the result to standard output.",
	0
    };
    register int j;

    for (j = 0; tbl[j] != 0; j++)
	fprintf(stderr, "%s\n", tbl[j]);
    exit(EXIT_FAILURE);
}

static char *
StrAlloc(const char *s)		/* patch: not everyone has 'strdup()' */
{
    char *d = malloc(strlen(s) + 1);

    if (d == 0)
	failed("malloc");
    return strcpy(d, s);
}

static void
Remove(int offset, LIST * list, char *name)
{
    int n = offset;

    TRACE((stderr, "Remove %d:%s\n", offset, name));
    while (list[n].nn != 0) {
	if (!Compare(list[n].nn, name)) {
	    do {
		list[n] = list[n + 1];
	    }
	    while (list[++n].nn != 0);
	    n = offset - 1;
	}
	n++;
    }
}

static int
Append(int offset, LIST * list, char *name)
{
    int n = offset;
    LIST temp, save;

    TRACE((stderr, "Append %d:%s\n", offset, name));
    temp.nn = name;

    do {
	n++;
	save = list[n];
	list[n] = temp;
	temp = save;
    }
    while (list[n].nn != 0);

    return offset + 1;
}

static int
exists(LIST * entry)
{
    TRACE((stderr, "exists(%s)\n", entry->nn));
    if (stat(entry->nn, &(entry->sb)) >= 0) {
	if ((entry->sb.st_mode & S_IFMT) == S_IFDIR) {
	    TRACE((stderr, "...directory\n"));
	    return TRUE;
	}
	if (allow_files
	    && (entry->sb.st_mode & S_IFMT) == S_IFREG) {
	    TRACE((stderr, "...file\n"));
	    return TRUE;
	}
    }
    TRACE((stderr, "...not found\n"));
    return FALSE;
}

int
main(int argc, char *argv[])
{
    const char *name = "PATH";
    char out_delim = PATHDELIM;
    int remove_duplicates = FALSE;
    size_t length = (size_t) argc;
    int operation = 'a';
    const char *where = 0;

    int c, point = 0;
    LIST *list;
    char *s;

    while ((c = getopt(argc, argv, "a:bdefn:prv")) != EOF) {
	switch (c) {
	case 'a':
	    where = optarg;
	    break;
	case 'b':
	    where = BLANK;
	    break;
	case 'd':
	    remove_duplicates = TRUE;
	    break;
	case 'e':
	    where = 0;
	    break;
	case 'f':
	    allow_files = TRUE;
	    break;
	case 'n':
	    name = optarg;
	    break;
	case 'p':
	    out_delim = ' ';
	    break;
	case 'r':
	    if (where == 0)
		where = BLANK;
	    operation = c;
	    break;
	case 'v':
	    out_delim = '\n';
	    break;
	default:
	    usage();
	}
    }

    /* Get the current path, make a corresponding list of strings.  Leave
     * enough room in the list to allow us to insert the arguments also.
     */
    if ((s = getenv(name)) == 0)
	s = StrAlloc(BLANK);
    else
	s = StrAlloc(s);	/* ...just in case someone else uses it */

    for (c = 0; s[c] != EOS; c++)
	if (s[c] == PATHDELIM)
	    length++;
    length += 3;
    list = (LIST *) calloc(length, sizeof(LIST));
    list[0].nn = StrAlloc(BLANK);	/* dummy entry, to simplify -b option */

    /* Split the environment variable into strings indexed in list[] */
    for (c = 1; *s != EOS; c++) {
	if (*s == PATHDELIM) {
	    list[c].nn = StrAlloc(".");
	} else {
	    list[c].nn = s;
	    while (*s != PATHDELIM && *s != EOS)
		s++;
	    if (*s == EOS)
		continue;
	    *s = EOS;
	}
	s++;
    }
    list[c].nn = 0;
    TRACE((stderr, "%s has %d entries\n", name, c));

    /* Find the list-entry after which we insert/remove entries */
    for (c = 0; list[c].nn != 0; c++) {
	if (where != 0 && !Compare(where, list[c].nn)) {
	    point = c;
	    break;
	}
    }
    if (list[c].nn == 0)
	point = c - 1;
    TRACE((stderr, "argc=%d, point=%d (%c)\n", argc, point, operation));

    /* Perform the actual insertion/removal */
    while (optind < argc) {
	s = argv[optind++];
	if (!Compare(s, "-"))
	    break;
	if (operation == 'r') {
	    Remove(point, list, s);
	} else {
	    point = Append(point, list, s);
	}
    }

    if (remove_duplicates) {
	/* Check to see if the directory exists. If not, remove it */
	for (c = 1; list[c].nn != 0; c++)
	    if (!exists(&list[c])) {
		Remove(c, list, list[c].nn);
		c--;
	    };
	/* Compare the inode & device numbers of the remaining items */
	for (c = 1; list[c].nn != 0; c++) {
	    int d;

	    for (d = c + 1; list[d].nn != 0; d++) {
		if (SameDir(&list[c], &list[d])) {
		    Remove(d, list, list[d].nn);
		    d--;
		}
	    }
	}
    }

    /* Finally, print the path */
    if (optind < argc) {
	size_t len = strlen(name) + 2;
	char *changed = 0;

	for (c = 1; list[c].nn != 0; c++)
	    len += 1 + strlen(list[c].nn);
	changed = malloc(len);
	strcpy(changed, name);
	for (c = 1; list[c].nn != 0; c++) {
	    sprintf(changed + strlen(changed), "%c%s",
		    (c > 1) ? PATHDELIM : '=',
		    list[c].nn);
	}
	fflush(stderr);
	fflush(stdout);
	putenv(changed);
#ifdef WIN32
	spawnvp(_P_WAIT, argv[optind], &argv[optind]);
#else
	execvp(argv[optind], &argv[optind]);
#endif
	fflush(stderr);
	fflush(stdout);
    } else {
	fflush(stderr);
	for (c = 1; list[c].nn != 0; c++) {
	    if (c > 1)
		(void) putchar(out_delim);
	    (void) fputs(list[c].nn, stdout);
	}
	(void) putchar('\n');
	fflush(stdout);
    }

    exit(EXIT_SUCCESS);
    /*NOTREACHED */
}
