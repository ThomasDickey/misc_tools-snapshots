#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define NORMAL 5000.0
static double usecs = NORMAL;

static double timer(void)
{
	struct timeval tv;
	gettimeofday(&tv, (struct timezone *)0);
	return tv.tv_sec * 1000000. + tv.tv_usec;
}

static void put_char(int c)
{
	double t1;
	char C = c;

	t1 = timer();
	write (1, &C, 1);
	while ((timer() - t1) < usecs)
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
			char *name = argv[n];
			if (*name == '-') {
				switch (*++name) {
				case '2': case '3':
				case '4': case '5':
				case '6': case '7':
				case '8': case '9':
					usecs = NORMAL * ((*name) - '0');
					printf("Threshold:%f\n", usecs);
					break;
				}
			} else {
				FILE *fp = fopen(name, "r");
				if (fp != 0) {
					slowcat(fp);
					fclose(fp);
				}
			}
		}
	} else
		slowcat(stdin);
	return 0;
}
