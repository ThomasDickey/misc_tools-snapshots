/*
 * $Id: td_getopt.h,v 1.3 2020/10/26 00:55:34 tom Exp $
 *
 * This header provides "getopt" for systems which lack that function.
 */

#ifndef TD_GETOPT_H_incl
#define TD_GETOPT_H_incl 1

#include <stdio.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(NEED_GETOPT_H)
#include <getopt.h>
#endif

#ifndef HAVE_GETOPT

#include <string.h>

#define ERR(s, c) \
	if (td_opterr) {\
		(void) fputs(argv[0], stderr);\
		(void) fputs(s, stderr);\
		(void) fputc(c, stderr);\
		(void) fputc('\n', stderr);\
	}

static int td_opterr = 1;
static int td_optind = 1;
static int td_optopt;
static char *td_optarg;

static int
td_getopt(int argc, char *argv[], const char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1) {
	if (td_optind >= argc ||
	    argv[td_optind][0] != '-' || argv[td_optind][1] == '\0')
	    return (EOF);
	else if (strcmp(argv[td_optind], "--") == 0) {
	    td_optind++;
	    return (EOF);
	}
    }
    td_optopt = c = argv[td_optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
	ERR(": illegal option -- ", c);
	if (argv[td_optind][++sp] == '\0') {
	    td_optind++;
	    sp = 1;
	}
	return ('?');
    }
    if (*++cp == ':') {
	if (argv[td_optind][sp + 1] != '\0')
	    td_optarg = &argv[td_optind++][sp + 1];
	else if (++td_optind >= argc) {
	    ERR(": option requires an argument -- ", c);
	    sp = 1;
	    return ('?');
	} else
	    td_optarg = argv[td_optind++];
	sp = 1;
    } else {
	if (argv[td_optind][++sp] == '\0') {
	    sp = 1;
	    td_optind++;
	}
	td_optarg = NULL;
    }
    return (c);
}
#define getopt(argc, argv, opts) td_getopt(argc, argv, opts)
#define opterr td_opterr
#define optind td_optind
#define optopt td_optopt
#define optarg td_optarg
#endif

#endif /* TD_GETOPT_H_incl */
