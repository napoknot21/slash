#include "../src/string.h"
#include "../src/vector.h"
#include "test_lexer.h"
#include "testlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int vector_tests()
{
	struct vector * svec = make_vector(
			sizeof(struct string), 
			(void (*)(void*)) destruct_string, 
			(void (*)(void*, void*)) copy_str
			);

	struct string * str = make_string("dtest");
		
	push_back(svec, str);
	push_back(svec, str);
	push_back(svec, str);

	free_string(str);

	vtrunc(svec, 0, 2);
	printf("%ld\n", svec->size);

	struct string * cstr = front(svec);	
	printf("cstr: %s\n", c_str(cstr));
	
	free_vector(svec);

	long iterations = 1 + rand() % 0xffff;

	struct vector *vec = make_vector(sizeof(long), NULL, NULL);
	int ass = 1;

	for (long k = 0; k < iterations; k++) {
		push_back(vec, &k);
		long el = *(long *)at(vec, vec->size - 1);

		ass &= el == k;

		if ((k & 2) == 0) {
			pop_back(vec);
			//long *pel = (long *)pop_back(vec);
			//ass &= *pel == el;
			//free(pel);
		}

		if (!ass)
			break;
	}

	clear(vec);
	ass &= !vec->size;
	free_vector(vec);

	return ass;
}

int string_tests()
{
	long ch_s = rand() % 0xff;
	char *ch = malloc(ch_s);

	for (size_t k = 0; k < (size_t)ch_s - 1; k++) {
		*(ch + k) = rand() % 0x80;
	}

	ch[ch_s - 1] = 0x0;

	struct string *str = make_string(ch);
	append(str, str);

	size_t alpha = rand() % (str->cnt->size - 1);
	size_t beta = alpha + rand() % (str->cnt->size - alpha);

	struct string *sub = substr(str, alpha, beta);

	for (size_t k = 0; k < beta - alpha; k++) {
		if (*at_str(str, alpha + k) != *at_str(sub, k)) {
			if (str != NULL)
				free_string(str);
			if (sub != NULL)
				free(sub);
			if (ch != NULL)
				free(ch);
			return ASSERT(0);
		}
	}
	if (str != NULL)
		free_string(str);
	if (sub != NULL)
		free_string(sub);
	free(ch);
	return ASSERT(1);
}

int main()
{
	srand(time(NULL));

	int bool = 1;
	bool &= test(vector_tests, "vector");
	bool &= test(string_tests, "string");
	bool &= test(test_token, "make_token");
	bool &= test(test_lex, "lex");

	return bool ? 0 : 1;
}
