static	const char Id[] = "$Id: unmap.c,v 1.3 1996/08/14 23:27:12 tom Exp $";

#include <stdio.h>
#include <ctype.h>

#define ESC '\\'

void escape(int c)
{
	putchar(ESC);
	putchar(c);
}

void unmap(FILE *fp)
{
	int	c;
	while ((c = fgetc(fp)) != EOF) {
		c &= 0xff;
		switch (c) {
		case '\033':
			putchar('\n');
			escape('E');
			break;
		case '\b':
			escape('b');
			break;
		case '\f':
			escape('f');
			break;
		case '\n':
			putchar('\n');
			escape('n');
			break;
		case '\r':
			escape('r');
			break;
		case '\t':
			escape('t');
			break;
		case '^':
			escape(c);
			break;
		case '\177':
			escape('?');
			break;
		default:
			if (c > 128) {
				printf("%c%03o", ESC, c);
			} else if (iscntrl(c)) {
				printf("^%c", c | '@');
			} else if (c < 128) {
				putchar(c);
			} else {
				printf("%c%03o", ESC, c);
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		int	n;
		for (n = 1; n < argc; n++) {
			FILE	*fp = fopen(argv[n], "r");
			if (fp != 0) {
				unmap(fp);
				fclose(fp);
			}
		}
	} else {
		unmap(stdin);
	}
	return 0;
}
