#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "testlib.h"
#include "../src/vector.h"

int f() 
{
	return ASSERT(1);
}
int g() 
{
	return ASSERT(0);
}

int vector_tests()
{
	srand(time(NULL));
	long iterations = rand() % 0xffff;

	vector * vec = make_vector(sizeof(long));

	for(long k = 0; k < iterations; k++)
	{
		int ass = 1;

		push_back(vec, &k);
		long el = *(long*) at(vec, vec->size - 1);

		ass &= el == k;

		if((k & 2) == 0)
		{
			long pel = *(long*) pop_back(vec);
			ass &= pel == el;
		}

		if(!ass) return ASSERT(0);
	}

	return ASSERT(1);
}

int main() 
{
	int val = 1;
	printf("Test Preview\n");
	val &= test(f, "f");
	val &= test(vector_tests, "vector");
	//val &= test(g,"g");
	return val ? 0 : 1;
}
