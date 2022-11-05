#include <stdio.h>
#include <time.h>

#include "testlib.h"

#define C_RED "\x1B[31m"
#define C_GREEN "\x1B[32m"
#define C_CLEAR "\x1B[0m"

int test(int (*f)(), const char *name)
{
	printf("[TEST] %s()...\n", name);
	const clock_t tic = clock();
	int res = f();
	const time_t toc = clock();
	const double delta = ((double)toc - tic) * 1000.0 / CLOCKS_PER_SEC;
	if (res)
	{
		printf("[%sPASSED%s] %s() in %f ms \n\n", C_GREEN, C_CLEAR, name, delta);
	}
	return res;
}

int assert(int b, int line, const char *filename)
{
	if (b)
	{
		return 1;
	}
	printf("[%sFAILED%s] at line %d in file %s\n\n", C_RED, C_CLEAR, line, filename);
	return 0;
}
