static const char Id[] = "$Id: hex.c,v 1.4 2002/02/16 21:07:20 tom Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define EOS '\0'

static void
to_text(char *dst, long value)
{
    unsigned long p;

    dst[0] = EOS;
    for (p = value; p != 0; p >>= 8) {
	unsigned char q = p & 0xff;
	char last[80];
	strcpy(last, dst);
	if (q < 128 && isprint(q)) {
	    int m = 0;
	    dst[m++] = q;
	    if (q == '\\')
		dst[m++] = q;
	    dst[m] = EOS;
	} else {
	    sprintf(dst, "\\%03o", q);
	}
	strcat(dst, last);
    }
}

static void
to_utf8(char *dst, long ch)
{
    static const unsigned byteMask = 0xBF;
    static const unsigned otherMark = 0x80;
    static const unsigned firstMark[] =
    {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

    int result[7], *ptr;
    int count = 0;

    if ((unsigned long) ch < 0x80)
	count = 1;
    else if ((unsigned long) ch < 0x800)
	count = 2;
    else if ((unsigned long) ch < 0x10000)
	count = 3;
    else if ((unsigned long) ch < 0x200000)
	count = 4;
    else if ((unsigned long) ch < 0x4000000)
	count = 5;
    else if ((unsigned long) ch <= 0x7FFFFFFF)
	count = 6;
    else {
	count = 3;
	ch = 0xFFFD;
    }
    ptr = result + count;
    switch (count) {
    case 6:
	*--ptr = (ch | otherMark) & byteMask;
	ch >>= 6;
	/* FALLTHRU */
    case 5:
	*--ptr = (ch | otherMark) & byteMask;
	ch >>= 6;
	/* FALLTHRU */
    case 4:
	*--ptr = (ch | otherMark) & byteMask;
	ch >>= 6;
	/* FALLTHRU */
    case 3:
	*--ptr = (ch | otherMark) & byteMask;
	ch >>= 6;
	/* FALLTHRU */
    case 2:
	*--ptr = (ch | otherMark) & byteMask;
	ch >>= 6;
	/* FALLTHRU */
    case 1:
	*--ptr = (ch | firstMark[count]);
	break;
    }
    while (count--) {
	sprintf(dst, "\\%03o", 0xff & *ptr++);
	dst += strlen(dst);
    }
}

int
main(int argc, char **argv)
{
    int n;

    for (n = 1; n < argc; n++) {
	char *next = argv[n];
	long value = 0;
	char *temp;
	char text_buf[BUFSIZ];
	char utf8_buf[BUFSIZ];

	do {
	    value = (value * 16) + strtol(next, &temp, 0);
	    next = *temp ? ++temp : temp;
	} while (*next);

	to_text(text_buf, value);
	to_utf8(utf8_buf, value);
	printf("%s: %ld %#lo %#lx text \"%s\" utf8 %s\n",
	       argv[n], value, value, value, text_buf, utf8_buf);
    }
    exit(0);
}
