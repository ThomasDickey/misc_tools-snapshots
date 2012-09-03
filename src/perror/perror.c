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
	temp = 0;
	if ((value = (int) strtol(argv[n], &temp, 0)) > 0
	    && temp != 0
	    && *temp == 0) {
	    errno = value;
	    perror(argv[0]);
	}
    }
    return 0;
}
