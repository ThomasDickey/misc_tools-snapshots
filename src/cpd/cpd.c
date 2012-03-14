/*
 * $Id: cpd.c,v 1.3 2012/03/13 17:40:18 tom Exp $
 */
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	<utime.h>
#include	<sys/types.h>
#include	<sys/stat.h>

static char *leaf;
static int touch;

static void
usage(void)
{
    fprintf(stderr, "usage: %s %s dst[...]\n",
	    leaf, touch ? "date" : "src");
    exit(1);
}

static void
wantdate(void)
{
    fprintf(stderr, "date is in form mmddhhmm[yy]\n");
    usage();
}

int
main(int argc, char *argv[])
{
    struct utimbuf timep;
    struct stat sb1;
    register int j;

    if (!(leaf = strrchr(argv[0], '/')))
	leaf = argv[0];
    touch = !strcmp(leaf, "touch");

    if (argc <= 1) {
	fprintf(stderr, "expected arguments\n");
	usage();
    }

    if (touch) {
	char *s = argv[1];
	int mon, day, hour, min, yy;
	time_t when = time(0);
	struct tm tm;

	tm = *localtime(&when);

	switch (strlen(s)) {
	case 8:
	    yy = tm.tm_year;
	    if (sscanf(s, "%02d%02d%02d%02d",
		       &mon, &day, &hour, &min) != 4)
		wantdate();
	    break;
	case 10:
	    if (sscanf(s, "%02d%02d%02d%02d%02d",
		       &mon, &day, &hour, &min, &yy) != 5)
		wantdate();
	    break;
	default:
	    wantdate();
	}

	tm.tm_mon = mon - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = 0;
	when = mktime(&tm);
	fprintf(stderr, "touch %s", ctime(&when));
	for (j = 2; j < argc; j++) {
	    timep.actime =
		timep.modtime = when;
	    if (utime(argv[j], &timep) < 0)
		perror(argv[j]);
	}
    } else {
	for (j = 2; j < argc; j++)
	    if (stat(argv[1], &sb1) >= 0) {
		timep.actime = sb1.st_atime;
		timep.modtime = sb1.st_mtime;
		if (utime(argv[j], &timep) < 0)
		    perror(argv[j]);
	    }
    }
    return (EXIT_SUCCESS);
}
