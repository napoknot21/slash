#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#define VECTOR_DEFAULT_CAPACITY 32

struct vector {
	size_t capacity;
	size_t size;
	size_t elem_s;

	void *data;
	void (*free)(void *);
};

/*
 * Construction/Destruction functions
 */

struct vector *make_vector(size_t, void (*)(void *));
void free_vector(struct vector *);

/*
 * Operation functions
 */

int push_back(struct vector *, void *);
void pop_back(struct vector *);

void reserve(struct vector *, size_t);
void clear(struct vector *);

/*
 * Access function
 */

void *at(struct vector *, size_t);
void *front(struct vector *);
void *back(struct vector *);

#endif
