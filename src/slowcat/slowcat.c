/*
 * $Id: slowcat.c,v 1.8 2026/08/18 23:47:25 tom Exp $
 *
 * Author:	T.E.Dickey
 * Created:	14 Jan 1997
 * Purpose:	write a file to standard output SLOWLY
 *
 * Changes:
 *		18 Aug 2026, set output to raw mode
 *		31 Aug 1997, allow option on pipe
 *		06 Jul 1997, write directly w/o buffering.
 *			     add -<number> options
 */
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>

#define NORMAL 5000.0
static double usecs = NORMAL;

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

static int first_file;
static struct termios tty_data;

static void
restore_tty(void)
{
    if (first_file == 0) {
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &tty_data);
    }
}

static void
setup_tty(void)
{
    first_file = -1;
    if (tcgetattr(STDOUT_FILENO, &tty_data) == 0) {
	struct termios raw_data = tty_data;

	raw_data.c_oflag &= (unsigned) ~ONLCR;
	raw_data.c_oflag &= (unsigned) ~OCRNL;
	raw_data.c_oflag &= (unsigned) ~OPOST;
	raw_data.c_oflag &= (unsigned) ~XTABS;

	if (tcsetattr(STDOUT_FILENO, TCSADRAIN, &raw_data) == 0)
	    first_file = 0;
    }
}

static double
timer(void)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *) 0);
    return (double) tv.tv_sec * 1000000. + (double) tv.tv_usec;
}

static void
put_char(int c)
{
    double t1;
    char C = (char) c;

    t1 = timer();
    if (write(1, &C, 1) < 0)
	exit(EXIT_FAILURE);
    while ((timer() - t1) < usecs) {
	;
    }
}

static void
slowcat(FILE *fp)
{
    int c;

    if (first_file > 0) {
	setup_tty();
	atexit(restore_tty);
    }

    while ((c = fgetc(fp)) != EOF)
	put_char(c);
}

int
main(int argc, char *argv[])
{
    int n;
    int ok = 0;

    if (!isatty(STDOUT_FILENO)) {
	fprintf(stderr, "Output is not a tty\n");
	return EXIT_FAILURE;
    }

    if (argc > 1) {
	for (n = 1; n < argc; n++) {
	    char *name = argv[n];
	    if (*name == '-') {
		switch (*++name) {
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    usecs = NORMAL * ((*name) - '0');
		    printf("Threshold:%f\n", usecs);
		    break;
		}
	    } else {
		FILE *fp = fopen(name, "r");
		ok++;
		if (fp != NULL) {
		    slowcat(fp);
		    fclose(fp);
		}
	    }
	}
	if (!ok)
	    slowcat(stdin);
    } else
	slowcat(stdin);
    return EXIT_SUCCESS;
}
