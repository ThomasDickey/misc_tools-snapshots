/*
 * Title:	map.c
 * Author:	T.E.Dickey
 * Created:	09 Jun 1997
 * Function:	Translate one or more files containing printing characters
 *		into nonprinting form.
 */

static const char Id[] = "$Id: map.c,v 1.1 1997/06/09 10:57:36 tom Exp $";

#include "unmap.h"

int main(int argc, char **argv)
{
	if (argc > 1) {
		int	n;
		for (n = 1; n < argc; n++) {
			FILE	*fp = fopen(argv[n], "r");
			if (fp != 0) {
				map(fp, stdout);
				fclose(fp);
			}
		}
	} else {
		map(stdin, stdout);
	}
	return 0;
}
