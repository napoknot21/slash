#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "testlib.h"
#include "../src/vector.h"
#include "test_lexer.h"
#include "../src/string.h"

int vector_tests()
{
	long iterations = 1 + rand() % 0xffff;

	vector *vec = make_vector(sizeof(long), NULL);
	int ass = 1;

	for (long k = 0; k < iterations; k++)
	{
		push_back(vec, &k);
		long el = *(long *)at(vec, vec->size - 1);

		ass &= el == k;

		if ((k & 2) == 0)
		{
			long *pel = (long *)pop_back(vec);
			ass &= *pel == el;
			free(pel);
		}

		if (!ass)
			break;
	}

	clear(vec);
	ass &= !vec->size;
	free(vec);
	return ass;
}

int string_tests()
{
	long ch_s = rand() % 0xff;
	char *ch = malloc(ch_s);

	for (size_t k = 0; k < (size_t)ch_s - 1; k++)
	{
		*(ch + k) = rand() % 0x80;
	}

	ch[ch_s - 1] = 0x0;

	string *str = make_string(ch);
	append(str, str);

	size_t alpha = rand() % (str->cnt->size - 1);
	size_t beta = alpha + rand() % (str->cnt->size - alpha);

	string *sub = substr(str, alpha, beta);

	for (size_t k = 0; k < beta - alpha; k++)
	{
		if (*at_str(str, alpha + k) != *at_str(sub, k))
		{
			free(str);
			free(sub);
			free(ch);
			return ASSERT(0);
		}
	}
	free(str);
	free(sub);
	free(ch);
	return ASSERT(1);
}

int main()
{
	srand(time(NULL));

	int bool = 1;
	bool &= test(vector_tests, "vector");
	bool &= test(string_tests, "string");
	bool &= test(test_token, "token_new");
	bool &= test(test_lex, "lex");

	return bool ? 0 : 1;
}
