/*
 * Title:	unmap.c
 * Author:	T.E.Dickey
 * Created:	15 Dec 1995
 * Function:	Translate one or more files containing nonprinting characters
 *		into visible form.
 */

static const char Id[] = "$Id: unmap.c,v 1.5 1997/06/09 10:46:58 tom Exp $";

#include "unmap.h"

int main(int argc, char **argv)
{
	if (argc > 1) {
		int	n;
		for (n = 1; n < argc; n++) {
			FILE	*fp = fopen(argv[n], "r");
			if (fp != 0) {
				unmap(fp, stdout);
				fclose(fp);
			}
		}
	} else {
		unmap(stdin, stdout);
	}
	return 0;
}
