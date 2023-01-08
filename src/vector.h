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
	void (*copy)(const void *, void *);
};

/*
 * Construction/Destruction functions
 */

struct vector *make_vector(size_t, void (*)(void *), void (*)(void*, void*));
void destruct_vector(struct vector *);
void free_vector(struct vector *);
void copy_vec(const struct vector *, struct vector *);

/*
 * Operation functions
 */

int push_back(struct vector *, const void *);
int append_vec(struct vector *u, struct vector *v);
void pop_back(struct vector *);
void *peek_pop(struct vector *vec, void *dst);
void vtrunc(struct vector *, size_t, size_t);
void pop_back_n(struct vector *vec, size_t n);

void reserve(struct vector *, size_t);
void clear(struct vector *);

/*
 * Access function
 */

void *at(const struct vector *, size_t);
void *front(struct vector *);
void *back(struct vector *);

#endif
