#include <time.h>
#include <sys/time.h>
#include <stdio.h>

static double timer(void)
{
	struct timeval tv;
	gettimeofday(&tv, (struct timezone *)0);
	return tv.tv_sec * 1000000. + tv.tv_usec;
}

static void put_char(int c)
{
	double t1;

	t1 = timer();
	putchar(c);
	while ((timer() - t1) < 5000.0)
		;
}

static void slowcat(FILE *fp)
{
	int c;
	while ((c = fgetc(fp)) != EOF)
		put_char(c);
}

int main(int argc, char *argv[])
{
	int n;

	if (argc > 1) {
		for (n = 1; n < argc; n++) {
			FILE *fp = fopen(argv[n], "r");
			if (fp != 0) {
				slowcat(fp);
				fclose(fp);
			}
		}
	} else
		slowcat(stdin);
	return 0;
}
