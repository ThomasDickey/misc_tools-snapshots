/*
 * Title:	unmap_s.c
 * Author:	T.E.Dickey
 * Created:	15 Dec 1995
 * Function:	Translate one or more files containing nonprinting characters
 *		into visible form.
 */

static const char Id[] = "$Id: unmap_s.c,v 1.5 2004/12/31 21:08:45 tom Exp $";

#include "unmap.h"

#define UCS_REPL 0xfffd

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

static unsigned utf_char = 0;
static int utf_count;
static int utf_data;

static int
decode_utf8(unsigned ch)
{
    int result = 0;

    do {
	/* Combine UTF-8 into Unicode */
	if (ch < 0x80) {
	    /* We received an ASCII character */
	    if (utf_count > 0) {
		utf_data = UCS_REPL;	/* prev. sequence incomplete */
	    } else {
		utf_data = ch;
	    }
	    result = 1;
	    break;
	} else if (ch < 0xc0) {
	    /* We received a continuation byte */
	    if (utf_count < 1) {
		/*
		 * We received a continuation byte before receiving a sequence
		 * state.  Or an attempt to use a C1 control string.  Either
		 * way, it is mapped to the replacement character.
		 */
		utf_data = UCS_REPL;	/* ... unexpectedly */
		result = 1;
		break;
	    } else {
		/* Check for overlong UTF-8 sequences for which a shorter
		 * encoding would exist and replace them with UCS_REPL.
		 * An overlong UTF-8 sequence can have any of the following
		 * forms:
		 *   1100000x 10xxxxxx
		 *   11100000 100xxxxx 10xxxxxx
		 *   11110000 1000xxxx 10xxxxxx 10xxxxxx
		 *   11111000 10000xxx 10xxxxxx 10xxxxxx 10xxxxxx
		 *   11111100 100000xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		 */
		if (!utf_char && !((ch & 0x7f) >> (7 - utf_count))) {
		    utf_char = UCS_REPL;
		}
		/* characters outside UCS-2 become UCS_REPL */
		if (utf_char > 0x03ff) {
		    /* value would be >0xffff */
		    utf_char = UCS_REPL;
		} else {
		    utf_char <<= 6;
		    utf_char |= (ch & 0x3f);
		}
		if ((utf_char >= 0xd800 &&
		     utf_char <= 0xdfff) ||
		    (utf_char == 0xfffe) ||
		    (utf_char == 0xffff)) {
		    utf_char = UCS_REPL;
		}
		utf_count--;
		if (utf_count == 0) {
		    utf_data = utf_char;
		    result = 1;
		    break;
		}
	    }
	} else {
	    /* We received a sequence start byte */
	    if (utf_count > 0) {
		utf_data = UCS_REPL;	/* prev. sequence incomplete */
		result = 1;
		break;
	    }
	    if (ch < 0xe0) {
		utf_count = 1;
		utf_char = (ch & 0x1f);
		if (!(ch & 0x1e))
		    utf_char = UCS_REPL;	/* overlong sequence */
	    } else if (ch < 0xf0) {
		utf_count = 2;
		utf_char = (ch & 0x0f);
	    } else if (ch < 0xf8) {
		utf_count = 3;
		utf_char = (ch & 0x07);
	    } else if (ch < 0xfc) {
		utf_count = 4;
		utf_char = (ch & 0x03);
	    } else if (ch < 0xfe) {
		utf_count = 5;
		utf_char = (ch & 0x01);
	    } else {
		utf_data = UCS_REPL;
		result = 1;
		break;
	    }
	}
    } while (0);

    return (result);
}

int
unmap(FILE *ifp, FILE *ofp, int utf8)
{
    int c;
    int last = 1;
    int count = 0;
    char temp[BUFSIZ];

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
	    if (utf8 && c >= 128) {
		if (decode_utf8((unsigned) c) > 0) {
		    sprintf(temp, "%cu%04X", BACKSLASH, utf_data);
		    utf_data = 0;
		    utf_count = 0;
		}
	    } else {
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
	    }
	    for (c = 0; temp[c]; c++) {
		count += put_ch(ofp, temp[c]);
	    }
	}
    }
    count += put_ch(ofp, '\n');

    return count;
}
