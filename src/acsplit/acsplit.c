/*
 * $Id: acsplit.c,v 1.13 2019/12/31 18:45:15 tom Exp $
 *
 * Title:	acsplit.c - split aclocal.m4
 * Author:	T.E.Dickey
 * Created:	28 Aug 1997
 * Function:	Splits an autoconf macro file (usually aclocal.m4) into
 *		a parent file which can be reprocessed by acmerge, and
 *		the macros in a subdirectory.
 *
 * Assumes:
 * we do not use changequote().  Since autoconf 2.5x breaks changequote(), that
 * is not a bad assumption anymore.
 *
 * AC_DEFUN or define's begin a line.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define VERBOSE(n) if(verbose > n) printf
#define isname(c) ((isalnum(c) || (c) == '_'))

#define remember(path,num) cur_path = path; cur_line = num

#define L_BRACK '['
#define R_BRACK ']'

#define L_PAREN '('
#define R_PAREN ')'

static const char *target = "AcSplit";
static int verbose;
static const char *cur_path;
static int cur_line;

static void
failed(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static char *
where(void)
{
    static char result[256];
    sprintf(result, "%s:%d:", cur_path, cur_line);
    return result;
}

static char *
skip_blanks(char *s)
{
    while (isspace(*s))
	s++;
    return s;
}

static char *
skip_punct(char *s)
{
    while (*s != '\0' && strchr("[()]", *s))
	s++;
    return s;
}

static char *
skip_comment(char *s)
{
    s = skip_blanks(s);
    if (!strncmp(s, "dnl", 3)) {
	s += 3;
    } else if (!strncmp(s, "#", 1)) {
	s += 1;
    } else {
	return 0;
    }
    if (isspace(*s) || ispunct(*s))
	return skip_blanks(s);
    return 0;
}

static int
is_dashes(char *line)
{
    static const char dashes[] = "------------------------------------------------------------------------";
    line = skip_blanks(line);
    line = skip_comment(line);
    if (line != 0) {
	return (!strncmp(line, dashes, sizeof(dashes) - 1));
    }
    return 0;
}

static char *
match(char *s, const char *name)
{
    char *base = s;

    while (*s == *name) {
	s++, name++;
    }
    return (!isname(*s) && (s != base)) ? s : 0;
}

/* AC_DEFUN([CF_ADD_INCDIR] */
static int
defined(char *line, char *name)
{
    char *s;

    line = skip_blanks(line);
    if (((s = match(line, "AC_DEFUN")) != 0)
	|| ((s = match(line, "define")) != 0)) {
	char temp[BUFSIZ];
	char *t = temp;

	s = skip_blanks(s);
	s = skip_punct(s);
	s = skip_blanks(s);
	while (isname(*s))
	    *t++ = *s++;
	*t = 0;
	if (t != temp) {
	    strcpy(name, temp);
	    return 1;
	}
    }
    return 0;
}

static char *
my_temp(char *dst)
{
    sprintf(dst, "%s/%s.out", target, target);
    return dst;
}

static FILE *
finish(FILE *ofp, char *name)
{
    char temp[BUFSIZ];

    my_temp(temp);
    if (ofp != 0)
	fclose(ofp);

    if (*name != 0) {
	char dst[BUFSIZ];

	VERBOSE(1) ("%s finish %s\n", where(), name);

	sprintf(dst, "%s/%s", target, name);
	rename(temp, dst);
	*name = 0;

	remove(temp);
    }
    if ((ofp = fopen(temp, "w")) == 0)
	failed(temp);
    return ofp;
}

static FILE *
append(FILE *ofp, FILE *hdr)
{
    char temp[BUFSIZ];
    char bfr[BUFSIZ];

    if (ofp != 0) {

	fclose(ofp);
	my_temp(temp);
	VERBOSE(1) ("%s append %s\n", where(), temp);

	ofp = fopen(temp, "r");
	while (fgets(bfr, sizeof(bfr), ofp)) {
	    VERBOSE(2) ("*...%s", bfr);
	    fputs(bfr, hdr);
	}
	fclose(ofp);

	remove(temp);
	if ((ofp = fopen(temp, "w")) == 0)
	    failed(temp);
    }
    return ofp;
}

#define WARN(msg) fprintf(stderr, "%s:%d:%s\n", path, lineno, msg)

static void
acsplit(const char *path)
{
    FILE *hdr;
    FILE *ifp = fopen(path, "r");
    FILE *ofp;
    char name[BUFSIZ];
    char bfr[BUFSIZ];
    int content = 0;
    int level = 0;
    int quote = 0;
    int n;
    int lineno = 0;
    int last = 0;

    if (ifp == 0)
	failed(path);
    mkdir(target, 0777);

    sprintf(name, "%s.in", path);
    remove(name);
    if ((hdr = fopen(name, "w")) == 0)
	failed(name);

    name[0] = 0;
    ofp = finish((FILE *) 0, name);

    while (fgets(bfr, sizeof(bfr), ifp) != 0) {
	remember(path, ++lineno);
	VERBOSE(2) ("%5d.  %s", content, bfr);
	if (level == 0 && !quote && (last || is_dashes(bfr))) {
	    if (*name != 0) {
		ofp = finish(ofp, name);
	    } else {
		ofp = append(ofp, hdr);
		if (content)
		    fputs(bfr, hdr);
	    }
	    content = 0;
	    last = 0;
	} else {
	    fputs(bfr, ofp);
	    content++;
	}
	if (level == 0 && !quote) {
	    if (defined(bfr, name)) {
		VERBOSE(0) ("%s start %s\n", where(), name);
		fprintf(hdr, "%s\n", name);
	    }
	}
	if (skip_comment(bfr) == 0) {	/* not a comment-line? */
	    int found = 0;
	    for (n = 0; bfr[n]; ++n) {
		switch (bfr[n]) {
		case L_BRACK:
		    ++quote;
		    break;
		case R_BRACK:
		    if (quote) {
			--quote;
			found = 1;
		    } else {
			WARN("Unbalanced bracket");
		    }
		    break;
		case L_PAREN:
		    if (!quote)
			++level;
		    break;
		case R_PAREN:
		    if (!quote) {
			if (level) {
			    --level;
			    found = 1;
			} else {
			    WARN("Unbalanced parenthesis");
			}
		    }
		    break;
		}
	    }
	    if (found && !level && !quote && *name) {
		last = 1;
		VERBOSE(2) ("will finish '%s'\n", name);
	    }
	}
    }
    ofp = finish(ofp, name);
    fclose(hdr);
    remove(my_temp(name));
}

static void
usage(void)
{
    fprintf(stderr, "Usage: acsplit [-v] [aclocal.m4]\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int n;
    int found = 0;

    if (argc > 1) {
	for (n = 1; n < argc; n++) {
	    char *parm = argv[n];
	    if (*parm == '-') {
		while (*++parm) {
		    switch (*parm) {
		    case 'v':
			++verbose;
			break;
		    default:
			usage();
			break;
		    }
		}
	    } else {
		acsplit(parm);
		found++;
	    }
	}
    }
    if (!found)
	acsplit("aclocal.m4");

    return EXIT_SUCCESS;
}
