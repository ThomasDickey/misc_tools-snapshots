#ifndef	NO_IDENT
static	char	Id[] = "$Header: /users/source/archives/misc_tools.vcs/src/newpath/RCS/newpath.c,v 1.3 1994/06/02 17:14:06 dickey Exp $";
#endif

/*
 * Title:	newpath.c
 * Author:	T.E.Dickey
 * Created:	02 Jun 1994
 * Modified:
 *
 * Function:
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	PATHDELIM ':'

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

#define	TRACE(p) /*fprintf p; */

typedef	struct	{
	char		*nn;
	struct	stat	sb;
	} LIST;

extern	int	getopt();
extern	int	optind;
extern	char	*optarg;

static
void	usage()
{
	static	char	*tbl[] = {
	"Usage: newpath [options [directories]]",
	"",
	"Echos the PATH environment variable with the given directories added",
	"(the default) or removed.",
	"",
	"Options:",
	"    -a NAME modify path after given NAME",
	"    -b      put new arguments before existing path",
	"    -d      remove duplicates/non-directory items",
	"    -e      put new arguments after end of path",
	"    -n NAME specify environment-variable to use (default: PATH)",
	"    -p      print in C-shell form",
	"    -r      remove arguments from path",
	0
	};
	register int	j;
	for (j = 0; tbl[j] != 0; j++)
		fprintf(stderr, "%s\n", tbl[j]);
	exit(EXIT_FAILURE);
}

static
void	Remove (offset, list, name)
	int	offset;
	LIST	*list;
	char	*name;
{
	int	n = offset;

	TRACE((stderr, "Remove %d:%s\n", offset, name))
	while (list[n].nn != 0) {
		if (!strcmp(list[n].nn, name)) {
			do {
				list[n] = list[n+1];
			} while (list[++n].nn != 0);
			n = offset - 1;
		}
		n++;
	}
}

static
int	Append (offset, list, name)
	int	offset;
	LIST	*list;
	char	*name;
{
	int	n = offset;
	LIST	temp, save;

	TRACE((stderr, "Append %d:%s\n", offset, name))
	temp.nn = name;

	do {
		n++;
		save    = list[n];
		list[n] = temp;
		temp    = list[n+1];
		list[n+1] = save;
	} while (list[n].nn != 0);

	return offset + 1;
}

int	main(argc, argv)
	char	*argv[];
{
	char	*name = "PATH";
	char	out_delim = PATHDELIM;
	int	remove_duplicates = FALSE;
	int	length = argc;
	int	operation = 'a';
	char	*where = 0;

	int	c, point;
	LIST	*list;
	char	*s;

	while ((c = getopt(argc, argv, "a:bden:pr")) != EOF) {
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
		default:
			usage();
		}
	}

	/* Get the current path, make a corresponding list of strings.  Leave
	 * enough room in the list to allow us to insert the arguments also.
	 */
	if ((s = getenv(name)) == 0)
		s = BLANK;
	s = strdup(s);		/* ...just in case someone else uses it */
	for (c = 0; s[c] != EOS; c++)
		if (s[c] == PATHDELIM)
			length++;
	length += 3;
	list = (LIST *)calloc(length, sizeof(LIST));
	list[0].nn = BLANK;	/* dummy entry, to simplify -b option */

	/* Split the environment variable into strings indexed in list[] */
	for (c = 1; *s != EOS; c++) {
		if (*s == PATHDELIM) {
			list[c].nn = strdup(".");
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
	TRACE((stderr, "%s has %d entries\n", name, c))

	/* Find the list-entry after which we insert/remove entries */
	for (c = 0; list[c].nn != 0; c++) {
		if (where != 0 && !strcmp(where, list[c].nn)) {
			point = c;
			break;
		}
	}
	if (list[c].nn == 0)
		point = c - 1;
	TRACE((stderr, "argc=%d,, point=%d (%c)\n", argc, point, operation))

	/* Perform the actual insertion/removal */
	while (optind < argc) {
		s = argv[optind++];
		if (operation == 'r') {
			Remove(point, list, s);
		} else {
			point = Append(point, list, s);
		}
	}

	if (remove_duplicates) {
		/* Check to see if the directory exists. If not, remove it */
		for (c = 1; list[c].nn != 0; c++)
			if (stat(list[c].nn, &(list[c].sb)) < 0
			 || (list[c].sb.st_mode & S_IFMT) != S_IFDIR) {
				Remove(c, list, list[c].nn);
				c--;
			}
		;
		/* Compare the inode & device numbers of the remaining items */
		for (c = 1; list[c].nn != 0; c++) {
			int	d;
			for (d = c+1; list[d].nn != 0; d++) {
				if (list[c].sb.st_ino == list[d].sb.st_ino
				 && list[c].sb.st_dev == list[d].sb.st_dev) {
					Remove(d, list, list[d].nn);
					d--;
				}
			}
		}
	}

	/* Finally, print the path */
	for (c = 1; list[c].nn != 0; c++) {
		if (c > 1)
			(void)putchar(out_delim);
		(void)fputs(list[c].nn, stdout);
	}
	(void)putchar('\n');
	fflush(stdout);

	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
}
