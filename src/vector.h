#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#define VECTOR_DEFAULT_CAPACITY 32

typedef struct vector
{
	size_t capacity;
	size_t size;
	size_t elem_s;

	void * data;

} vector;

vector * make_vector(size_t);

int push_back(vector *, void *);
void * pop_back(vector *);
void * at(vector *, size_t);

void clear(vector *);

void free_vector(vector *);

#endif
