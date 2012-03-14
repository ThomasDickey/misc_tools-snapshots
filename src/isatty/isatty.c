/*
 * $Id: isatty.c,v 1.2 2012/03/13 17:42:30 tom Exp $
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int
main(void)
{
    int code = ((isatty(fileno(stdin)) && isatty(fileno(stdout)))
		? EXIT_SUCCESS
		: EXIT_FAILURE);
    return code;
}
