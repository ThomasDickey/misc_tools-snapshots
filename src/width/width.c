#ifndef NO_IDENT
static const char Id[] = "$Id: width.c,v 1.3 1996/11/21 15:53:18 dickey Exp $";
#endif

/*
 * Title:	width.c
 * Author:	T.Dickey
 * Created:	21 Nov 1996 (from earlier version)
 * Function:	Display lines from a text file that are longer than a given
 *		threshold (usually 80).
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

extern int optind;
extern char *optarg;

static	int	opt_width  = -1;
static	int	opt_tabs   = -1;
static	int	opt_number = 0;
static	int	max_width  = 0;

static void failed(char *s)
{
	perror(s);
	exit(1);
}

/* report lines that are wider than the threshold */
static void report_width(char *name, int lineno, int column, char *buffer)
{
	int doit = 0;

	if (opt_width > 0)
	{
		if (column > opt_width)
			doit = 1;
	}
	if (column > max_width)
		max_width = column;

	if (doit)
	{
		if (opt_number)
			printf("%s:%d:", name, lineno);
		printf("%d:%s", column, buffer);
	}
}

static void width(char *name, FILE *fp)
{
	int	lineno = 1;
	int	column = 0;
	int	length = 0;
	int	c;
	char	buffer[BUFSIZ];

	while ((c = fgetc(fp)) != EOF)
	{
		if (length < sizeof(buffer)-1)
		{
			buffer[length++] = c;
			buffer[length] = '\0';
		}

		/* compute the effective line & column */
		if (c == '\n')
		{
			report_width(name, lineno++, column, buffer);
			column = 0;
			length = 0;
		}
		else if (opt_tabs > 0 && c == '\t')
		{
			column = opt_tabs * ((column / opt_tabs) + 1);
		}
		else if (isprint(c))
		{
			column++;
		}
		else if (opt_tabs <= 0)
		{
			column++;
		}

	}
	if (length > 0)
		report_width(name, lineno++, column, buffer);
}

static void usage(void)
{
	static const char *tbl[] = {
	 "Usage: width [options] [files]"
	,""
	,"Options:"
	,"  -4     set tabs to 4"
	,"  -8     set tabs to 8"
	,"  -n     show line-numbers of wide lines"
	,"  -t XX  set tabs to XX"
	,"  -w XX  set threshold to XX, showing all lines that are wider"
	,""
	,"(If you do not specify tabs, they will be counted as single-columns)"
	};
	int n;
	for (n = 0; n < sizeof(tbl)/sizeof(tbl[0]); n++)
		fprintf(stderr, "%s\n", tbl[n]);
	exit(1);
}

int main(int argc, char *argv[])
{
	int	c;

	while ((c = getopt(argc, argv, "w:nt:48")) != EOF)
	{
		switch (c)
		{
		case 'w':
			opt_width = atoi(optarg);
			break;
		case 'n':
			opt_number = 1;
			break;
		case '4':
			opt_tabs = 4;
			break;
		case '8':
			opt_tabs = 8;
			break;
		case 't':
			opt_tabs = atoi(optarg);
			break;
		default:
			usage();
		}
	}

	if (optind < argc)
	{
		while (optind < argc)
		{
			char *name = argv[optind++];
			FILE *fp = fopen(name, "r");
			if (fp == 0)
				failed(name);
			width(name, fp);
			(void) fclose(fp);
		}
	}
	else
	{
		width("<stdin>", stdin);
	}
	printf("%d\n", max_width);

	return 0;
}
