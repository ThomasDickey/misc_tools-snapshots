#include	<stdio.h>
#include	<string.h>
#include	<pwd.h>
extern	char	*getenv();

main(argc, argv)
char	*argv[];
{
	char	home[BUFSIZ],
		shell[BUFSIZ],
		user[BUFSIZ],
		*default_shell = "/bin/sh";
	register struct	passwd *q;

	(void)setuid(geteuid());
	(void)setgid(getegid());
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
