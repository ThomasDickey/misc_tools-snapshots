/*
 * $Id: sue.c,v 1.2 2000/12/28 13:37:03 tom Exp $
 */
#include	<stdlib.h>
#include	<unistd.h>	/* may define _POSIX_SAVED_IDS */
#include	<stdio.h>
#include	<string.h>
#include	<pwd.h>

int main(int argc, char *argv[])
{
	char	home[BUFSIZ],
		shell[BUFSIZ],
		user[BUFSIZ],
		*default_shell = "/bin/sh";
	register struct	passwd *q;

#ifndef _POSIX_SAVED_IDS
	/*
	 * This works _everywhere_ except for glibc 2.1.3, which is broken.
	 */
	(void)setuid(geteuid());
	(void)setgid(getegid());
#else
	/*
	 * For the moment (2000/12/28), no one's broken the "old" behavior.
	 */
	(void)setreuid(geteuid(), geteuid());
	(void)setregid(getegid(), getegid());
#endif
	strcpy(home,  "HOME=");
	strcpy(shell, "SHELL=");
	strcpy(user,  "USER=");
	if (q = getpwuid(getuid())) {
		if (q->pw_shell && q->pw_shell[0])
			default_shell = q->pw_shell;
		strcat(home,  q->pw_dir);
		strcat(shell, default_shell);
		strcat(user,  q->pw_name);
	} else {
		strcat(home,  ".");
		strcat(shell, default_shell);
		strcat(user,  getenv("LOGNAME"));
	}
	putenv(home);
	putenv(shell);
	putenv(user);
	if (argc > 1) {
		execvp(argv[1], argv+1);
		perror(argv[1]);
	} else
		system(default_shell);
	exit(0);
}
