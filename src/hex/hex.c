#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define EOS '\0'

int main(int argc, char **argv)
{
	int	n;

	for (n = 1; n < argc; n++) {
		char	*next = argv[n];
		long	value = 0;
		unsigned long p;
		char	*temp;
		char	string[80];

		do {
			value = (value * 16) + strtol(next, &temp, 0);
			next = *temp ? ++temp : temp;
		} while (*next);

		string[0] = EOS;
		for (p = value; p != 0; p >>= 8) {
			unsigned char q = p & 0xff;
			char last[80];
			strcpy(last, string);
			if (isascii(q) && isprint(q)) {
				int	m = 0;
				string[m++] = q;
				if (q == '\\')
					string[m++] = q;
				string[m] = EOS;
			} else {
				sprintf(string, "\\%03o", q);
			}
			strcat(string, last);
		}
		printf("%s: %ld %#lo %#lx \"%s\"\n",
			argv[n], value, value, value, string);
	}
	exit(0);
}
