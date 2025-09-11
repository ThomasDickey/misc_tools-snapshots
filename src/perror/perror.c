/*
 * $Id: perror.c,v 1.4 2025/09/10 20:40:21 tom Exp $
 *
 * Title:	perror.c
 * Author:	T.Dickey
 * Created:	08 Jul 2000
 * Function:	print system messages for errno values.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
    int n;
    int value;
    char *temp;

    for (n = 1; n < argc; n++) {
	temp = NULL;
	if ((value = (int) strtol(argv[n], &temp, 0)) > 0
	    && temp != NULL
	    && *temp == 0) {
	    errno = value;
	    perror(argv[0]);
	} else {
	    if (errno == 0)
		errno = EINVAL;
	    perror(argv[0]);
	    return EXIT_FAILURE;
	}
    }
    return EXIT_SUCCESS;
}
