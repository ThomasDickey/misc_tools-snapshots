/*
 * Title:	unmap_s.c
 * Author:	T.E.Dickey
 * Created:	15 Dec 1995
 * Function:	Translate one or more files containing nonprinting characters
 *		into visible form.
 */

static const char Id[] = "$Id: unmap_s.c,v 1.3 1997/06/10 00:10:59 tom Exp $";

#include "unmap.h"

static int
put_ch(FILE *ofp, int c)
{
	if (ofp != 0)
		fputc(c, ofp);
	return 1;
}

static int
escape(FILE *ofp, int c)
{
	int count = 0;
	count += put_ch(ofp, BACKSLASH);
	count += put_ch(ofp, c);
	return count;
}

int
unmap(FILE *ifp, FILE *ofp)
{
	int	c;
	int	last = 1;
	int	count = 0;
	char	temp[BUFSIZ];

	while ((c = fgetc(ifp)) != EOF) {
		c &= 0xff;
		last = 0;
		switch (c) {
		case ESCAPE:
			count += put_ch(ofp, '\n');
			count += escape(ofp, 'E');
			last = 1;
			break;
		case '\b':
			count += escape(ofp, 'b');
			break;
		case '\f':
			count += escape(ofp, 'f');
			break;
		case '\n':
			count += put_ch(ofp, '\n');
			count += escape(ofp, 'n');
			last = 1;
			break;
		case '\r':
			count += escape(ofp, 'r');
			break;
		case '\t':
			count += escape(ofp, 't');
			break;
		case '^':
			count += escape(ofp, c);
			break;
		case '\177':
			count += escape(ofp, '?');
			break;
		case BACKSLASH:
			count += escape(ofp, c);
			break;
		default:
			*temp = '\0';
			if (c >= 128) {
				sprintf(temp, "%c%03o", BACKSLASH, c);
			} else if (iscntrl(c)) {
				sprintf(temp, "^%c", c | '@');
			} else if (c < 128) {
				count += put_ch(ofp, c);
			} else {
				sprintf(temp, "%c%03o", BACKSLASH, c);
			}
			for (c = 0; temp[c]; c++) {
				count += put_ch(ofp, temp[c]);
			}
		}
	}
	count += put_ch(ofp, '\n');

	return count;
}
