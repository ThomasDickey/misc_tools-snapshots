#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int	main()
{
		if (isatty(fileno(stdin)) && isatty(fileno(stdout)))
			exit(0);
		exit(1);
}
