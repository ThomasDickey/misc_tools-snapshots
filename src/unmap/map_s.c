/*
 * $Id: map_s.c,v 1.5 2012/03/14 09:00:11 tom Exp $
 *
 * Title:	map.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Translate file containing printing characters into nonprinting
 *		form.
 */

#include "unmap.h"

#define isoctal(c) ('0' <= (c) && (c) <= '7')

static int
put_ch(FILE *ofp, int c)
{
    if (ofp != 0)
	fputc(c, ofp);
    return 1;
}

/*
 * Reference: The Unicode Standard 2.0
 *
 * No surrogates supported (we're storing only one 16-bit Unicode value per
 * cell).
 */
static void
utf8_outch(FILE *ofp, unsigned ch)
{
    static const unsigned byteMask = 0xBF;
    static const unsigned otherMark = 0x80;
    static const unsigned firstMark[] =
    {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

    int result[7], *ptr;
    int count = 0;

    if ((unsigned int) ch < 0x80)
	count = 1;
    else if ((unsigned int) ch < 0x800)
	count = 2;
    else if ((unsigned int) ch < 0x10000)
	count = 3;
    else if ((unsigned int) ch < 0x200000)
	count = 4;
    else if ((unsigned int) ch < 0x4000000)
	count = 5;
    else if ((unsigned int) ch <= 0x7FFFFFFF)
	count = 6;
    else {
	count = 3;
	ch = 0xFFFD;
    }
    ptr = result + count;
    switch (count) {
    case 6:
	*--ptr = (int) ((ch | otherMark) & byteMask);
	ch >>= 6;
	/* FALLTHRU */
    case 5:
	*--ptr = (int) ((ch | otherMark) & byteMask);
	ch >>= 6;
	/* FALLTHRU */
    case 4:
	*--ptr = (int) ((ch | otherMark) & byteMask);
	ch >>= 6;
	/* FALLTHRU */
    case 3:
	*--ptr = (int) ((ch | otherMark) & byteMask);
	ch >>= 6;
	/* FALLTHRU */
    case 2:
	*--ptr = (int) ((ch | otherMark) & byteMask);
	ch >>= 6;
	/* FALLTHRU */
    case 1:
	*--ptr = (int) (ch | firstMark[count]);
	break;
    }
    while (count--)
	put_ch(ofp, *ptr++);
}

int
map(FILE *ifp, FILE *ofp, int utf8)
{
    int c;
    int state = 0;
    int count = 0;
    int value = 0;
    int digit = 0;
    char buffer[5];

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
		case 'E':
		    c = ESCAPE;
		    break;
		case 'b':
		    c = '\b';
		    break;
		case 'f':
		    c = '\f';
		    break;
		case 'n':
		    c = '\n';
		    break;
		case 'r':
		    c = '\r';
		    break;
		case '^':
		    c = '^';
		    break;
		case 't':
		    c = '\t';
		    break;
		case '?':
		    c = '\177';
		    break;
		default:
		    if (c == 'u' && utf8) {
			state = 6;
		    } else if (isoctal(c)) {
			state = 3;
			value = c - '0';
		    }
		}
		if (state == 0)
		    count += put_ch(ofp, c);
		break;
	    case 2:
		switch (c) {
		case '?':
		    c = '\177';
		    break;
		default:
		    c &= 0x1f;
		    break;
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
	    case 6:
		if (isdigit(c)) {
		    buffer[digit++] = (char) c;
		    if (digit >= 4) {
			unsigned uvalue;

			buffer[digit] = 0;
			sscanf(buffer, "%X", &uvalue);
			utf8_outch(ofp, uvalue);
			state = 0;
		    }
		}
		break;
	    }
	}
    }
    if (state == 2)
	count += put_ch(ofp, value);
    return count;
}
