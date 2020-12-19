/*
 * $Id: realpath.c,v 1.6 2020/12/19 12:54:52 tom Exp $
 *
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
#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>		/* for MAXPATHLEN */
#endif

#ifndef HAVE_REALPATH
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <td_getline.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

static void
failed(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

#if !HAVE_REALPATH
static char *
realpath(const char *src, char *dst)
{
    char save[MAXPATHLEN];
    char temp[MAXPATHLEN];
    struct stat sb;

    src = strcpy(temp, src);
    if (stat(src, &sb) < 0)
	return 0;
    if ((sb.st_mode & S_IFMT) != S_IFDIR) {
	char *leaf = strrchr(src, '/');
	if (leaf == 0)
	    (void) strcpy(src, ".");
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

static void
do_path(const char *path)
{
    char resolved_path[MAXPATHLEN];

    if (realpath(path, resolved_path))
	puts(resolved_path);
    else
	failed(path);
}

int
main(int argc, char *argv[])
{
    int n;

    if (argc > 1) {
	for (n = 1; n < argc; n++)
	    do_path(argv[n]);
    } else if (isatty(fileno(stdin))) {
	do_path(".");
    } else {
	char *buffer = 0;
	size_t have = 0;
	while (getline(&buffer, &have, stdin) >= 0) {
	    int len = (int) strlen(buffer);
	    if (len > 0 && buffer[len - 1] == '\n')
		buffer[--len] = '\0';
	    if (len > 0)
		do_path(buffer);
	}
	free(buffer);
    }
    return (EXIT_SUCCESS);
}
