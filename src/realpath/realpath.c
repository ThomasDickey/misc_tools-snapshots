#ifndef	NO_IDENT
static	char	Id[] =
"$Header: /users/source/archives/misc_tools.vcs/src/realpath/RCS/realpath.c,v 1.2 1994/09/02 15:32:32 dickey Exp $";
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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>	/* for MAXPATHLEN */

/* mips: IRIX */
#if defined(sun) || defined(mips)
# define HAVE_REALPATH 1
#endif

#if !HAVE_REALPATH
#include <sys/types.h>
#include <sys/stat.h>
#endif

static
void failed(s)
	char	*s;
{
	perror(s);
	exit(1);
}

#if !HAVE_REALPATH
static
char *	realpath(src, dst)
	char	*src;
	char	*dst;
{
	char	save[MAXPATHLEN];
	char	temp[MAXPATHLEN];
	struct	stat	sb;

	src = strcpy(temp, src);
	if (stat(src, &sb) < 0)
		return 0;
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		char *leaf = strrchr(src, '/');
		if (leaf == 0)
			(void)strcpy(src, ".");
		else
			*leaf = '\0';
	}
	if (getcwd(save, sizeof(save))
	 && chdir(src) == 0
	 && getcwd(dst, MAXPATHLEN)
	 && chdir(save) == 0)
	 	return dst;
	return 0;
}
#endif

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
	/*NOTREACHED*/
}
