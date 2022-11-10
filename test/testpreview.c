#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "testlib.h"
#include "../src/vector.h"
#include "../src/string.h"

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

int string_tests()
{
	long ch_s = rand() % 0xff;
	char * ch = malloc(ch_s);

	for(size_t k = 0; k < ch_s - 1; k++)
	{
		*(ch + k) = rand() % 0x80;
	}

	ch[ch_s - 1] = 0x0;

	string * str = make_string(ch);		
	append(str, str);

	size_t alpha = rand() % (str->cnt->size - 1);
	size_t beta  = alpha + rand() % (str->cnt->size - alpha);

	string * sub = substr(str, alpha, beta);	
	
	for(size_t k = 0; k < beta - alpha; k++)
	{
		if(*at_str(str, alpha + k) != *at_str(sub, k))
		{
			return ASSERT(0);
		}
	}	

	return ASSERT(1);
}

int main() 
{
	srand(time(NULL));

	int val = 1;
	printf("Test Preview\n");
	val &= test(f, "f");

	val &= test(vector_tests, "vector");
	val &= test(string_tests, "string");

	//val &= test(g,"g");
	return val ? 0 : 1;
}
