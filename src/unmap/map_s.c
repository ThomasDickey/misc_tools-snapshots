/*
 * Title:	map.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Translate file containing printing characters into nonprinting
 *		form.
 */

static const char Id[] = "$Id: map_s.c,v 1.1 1997/06/10 00:03:29 tom Exp $";

#include "unmap.h"

#define isoctal(c) ('0' <= (c) && (c) <= '7')

static int
put_ch(FILE *ofp, int c)
{
	if (ofp != 0)
		fputc(c, ofp);
	return 1;
}

int map(FILE *ifp, FILE *ofp)
{
	int	c;
	int	state = 0;
	int	count = 0;
	int	value = 0;

	while ((c = fgetc(ifp)) != EOF) {
		c &= 0xff;
		if (isprint(c)) {
			switch (state) {
			case 0:
				if (c == BACKSLASH)
					state = 1;
				else if (c == '^')
					state = 2;
				else
					count += put_ch(ofp, c);
				break;
			case 1:
				state = 0;
				switch (c) {
				case 'E':	c = ESCAPE;	break;
				case 'b':	c = '\b';	break;
				case 'f':	c = '\f';	break;
				case 'n':	c = '\n';	break;
				case 'r':	c = '\r';	break;
				case '^':	c = '^';	break;
				case 't':	c = '\t';	break;
				case '?':	c = '\177';	break;
				default:
					if (isoctal(c)) {
						state = 3;
						value = c - '0';
					}
				}
				if (state == 0)
					count += put_ch(ofp, c);
				break;
			case 2:
				switch(c) {
				case '?':	c = '\177';	break;
				default:	c &= 0x1f;	break;
				}
				count += put_ch(ofp, c);
				state = 0;
				break;
			case 3:
				/* FALLTHRU */
			case 4:
				if (isoctal(c)) {
					value = value * 8 + (c - '0');
					state++;
					break;
				}
				/* FALLTHRU */
			case 5:
				state = 0;
				count += put_ch(ofp, value);
				ungetc(c, ifp);
				break;
			}
		}
	}
	if (state == 2)
		count += put_ch(ofp, value);
	return count;
}
