/*
 * $Id: sue.c,v 1.4 2012/03/14 09:03:25 tom Exp $
 *
 * Author:	T.E.Dickey
 * Created:	23 Apr 1990
 * Purpose:	provide a "su" command which preserves caller's environment.
 */
#include	<stdlib.h>
#include	<unistd.h>	/* may define _POSIX_SAVED_IDS */
#include	<stdio.h>
#include	<string.h>
#include	<pwd.h>

int
main(int argc, char *argv[])
{
    static char the_default_shell[] = "/bin/sh";
    char home[BUFSIZ];
    char shell[BUFSIZ];
    char user[BUFSIZ];
    char *default_shell = the_default_shell;
    int code = EXIT_SUCCESS;
    register struct passwd *q;

#ifndef _POSIX_SAVED_IDS
    /*
     * This works _everywhere_ except for glibc 2.1.3, which is broken.
     */
    (void) setuid(geteuid());
    (void) setgid(getegid());
#else
    /*
     * For the moment (2000/12/28), no one's broken the "old" behavior.
     */
    (void) setreuid(geteuid(), geteuid());
    (void) setregid(getegid(), getegid());
#endif
    strcpy(home, "HOME=");
    strcpy(shell, "SHELL=");
    strcpy(user, "USER=");
    if ((q = getpwuid(getuid())) != 0) {
	if (q->pw_shell && q->pw_shell[0])
	    default_shell = q->pw_shell;
	strcat(home, q->pw_dir);
	strcat(shell, default_shell);
	strcat(user, q->pw_name);
    } else {
	strcat(home, ".");
	strcat(shell, default_shell);
	strcat(user, getenv("LOGNAME"));
    }
    putenv(home);
    putenv(shell);
    putenv(user);
    if (argc > 1) {
	execvp(argv[1], argv + 1);
	perror(argv[1]);
    } else {
	if (system(default_shell))
	    code = EXIT_FAILURE;
    }
    exit(code);
}
