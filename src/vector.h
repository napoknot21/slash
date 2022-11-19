#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#define VECTOR_DEFAULT_CAPACITY 32

typedef struct vector {
	size_t capacity;
	size_t size;
	size_t elem_s;

	void *data;
	void (*free)(void *);

} vector;

/*
 * Construction/Destruction functions
 */

vector *make_vector(size_t, void (*)(void *));
void free_vector(vector *);

/*
 * Operation functions
 */

int push_back(vector *, void *);
void *pop_back(vector *);

void reserve(vector *, size_t);
void clear(vector *);

/*
 * Access function
 */

void *at(vector *, size_t);
void *front(vector *);
void *back(vector *);

#endif
