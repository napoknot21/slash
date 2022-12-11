#include "vector.h"
#include <string.h>

struct vector *make_vector(size_t elem_s, void (*free)(void *), void (*copy)(void *, void *))
{
	struct vector *vec = malloc(sizeof(struct vector));	

	vec->capacity = VECTOR_DEFAULT_CAPACITY;
	vec->size = 0;
	vec->elem_s = elem_s;

	vec->data = malloc(vec->elem_s * vec->capacity);
	vec->free = free;
	vec->copy = copy;
	
	return vec;
}

void copy_vec(const struct vector * vec, struct vector * cp)
{
	memset(cp, 0x0, sizeof(struct vector));

	cp->capacity = vec->capacity;	
	cp->size = 0;
	cp->elem_s = vec->elem_s;	

	cp->free = vec->free;
	cp->copy = vec->copy;
	
	cp->data = malloc(cp->elem_s * cp->capacity);	

	for(size_t k = 0; k < vec->size; k++) {
	
		push_back(cp, at(vec, k));

	}
}

void free_data(struct vector *vec)
{
	if (!vec->free) return;

	for (size_t k = 0; k < vec->size; k++) {
		
		void * el = (void*)((char*) vec->data + k * vec->elem_s);
		if(!el) continue;

		vec->free(el);	
	}
}

void destruct_vector(struct vector *vec)
{
	free_data(vec);
	free(vec->data);	
}

void free_vector(struct vector *vec)
{
	destruct_vector(vec);
	free(vec);
}

int push_back(struct vector *vec, void *data)
{
	if (vec->size >= vec->capacity) {
		reserve(vec, vec->capacity * 2);
		if (!vec->data)
			return 1;
	}	

	if(vec->copy)	
		vec->copy(data, (void*)((char*) vec->data + vec->size * vec->elem_s));	
	else			
		memmove((void*)((char*) vec->data + vec->size * vec->elem_s), data, vec->elem_s);

	vec->size++;

	return 0;
}

void pop_back(struct vector *vec)
{
	if (vec->size > 0) {

		void *e = (void*)((char*) vec->data + --vec->size * vec->elem_s);
		
		if(vec->free) 
			vec->free(e);
		else
			memset(e, 0x0, vec->elem_s);	
	
	}
}

void vtrunc(struct vector *vec, size_t from, size_t to)
{
	if(!vec->free)
		memset((void*)((char*) vec->data + from * vec->elem_s), 0x0, (to - from) * vec->elem_s);

	for(size_t k = from; k < to && vec->free; k++)
			vec->free((void*)((char*) vec->data + k * vec->elem_s));

	/*
	 * Move the tail data
	 */

	if(to < vec->size) {

		memmove(
			(void*)((char*) vec->data + from * vec->elem_s),
			(void*)((char*) vec->data + to * vec->elem_s),
			(vec->size - to) * vec->elem_s
		);

	}

	vec->size -= (to - from);
}

void *at(const struct vector *vec, size_t i)
{
	if (i >= vec->size)
		return NULL;

	return (void*)((char*) vec->data + i * vec->elem_s);
}

void reserve(struct vector *vec, size_t ncap)
{
	if (ncap <= vec->capacity)
		return;

	vec->capacity = ncap;
	void * tmp = realloc(vec->data, vec->elem_s * vec->capacity);	

	if (!tmp) {	
		free_data(vec);
		free(vec->data);
		vec->data = NULL;

		return;
	}
	
	vec->data = tmp;
}

void clear(struct vector *vec)
{
	free_data(vec);
	memset(vec->data, 0x0, vec->elem_s * vec->capacity);

	vec->size = 0;
}

void *front(struct vector *vec)
{
	return vec->data;
}

void *back(struct vector *vec)
{
	if (!vec->size)
		return NULL;

	return (void*)((char*) vec->data + (vec->size - 1) * vec->elem_s);
}
