#ifndef	NO_IDENT
static	char	Id[] =
"$Header: /users/source/archives/misc_tools.vcs/src/realpath/RCS/realpath.c,v 1.1 1994/06/17 12:09:21 dickey Exp $";
#endif

/*
 * Title:	realpath.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1994
 *
 * Function:	Uses the SunOS-specific 'realpath()' call to resolve the
 *		pathnames given as arguments into absolute pathnames.  This
 *		seems to work better than 'pwd' in the shell script idiom
 *
 *			FOO=`cd foo; pwd`
 *
 *		since SunOS does not always resolve the directory properly
 *		when the current directory is mounted.
 */
#include <stdio.h>
#include <string.h>
#include <sys/param.h>	/* for MAXPATHLEN */

static
void failed(s)
	char	*s;
{
	perror(s);
	exit(1);
}

static
void	do_path(path)
	char	*path;
{
	char	resolved_path[MAXPATHLEN];

	if (realpath(path, resolved_path))
		puts(resolved_path);
	else
		failed(path);
}

int	main(argc, argv)
	char	*argv[];
{
	int	n;

	if (argc > 1) {
		for (n = 1; n < argc; n++)
			do_path(argv[n]);
	} else if (isatty(fileno(stdin))) {
		do_path(".");
	} else {
		char	buffer[BUFSIZ];
		while (fgets(buffer, sizeof(buffer), stdin)) {
			int len = strlen(buffer);
			if (len > 0 && buffer[len-1] == '\n')
				buffer[--len] = '\0';
			if (len > 0)
				do_path(buffer);
		}
	}
	exit(0);
}
