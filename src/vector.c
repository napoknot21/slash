#include "vector.h"

#include <string.h>

struct vector *make_vector(size_t elem_s, void (*free)(void *))
{
	struct vector *vec = malloc(sizeof(struct vector));

	vec->capacity = VECTOR_DEFAULT_CAPACITY;
	vec->size = 0;
	vec->elem_s = elem_s;

	vec->data = malloc(elem_s * vec->capacity);
	vec->free = free;

	return vec;
}

void free_data(struct vector *vec)
{
	if (vec->free) {
		for (size_t k = 0; k < vec->size; k++) {
			vec->free(vec->data + k * vec->elem_s);
		}
	}
}

void free_vector(struct vector *vec)
{
	free_data(vec);
	free(vec->data);
	free(vec);
}

int push_back(struct vector *vec, void *data)
{
	if (vec->size >= vec->capacity) {
		reserve(vec, vec->capacity * 2);
		if (!vec->data)
			return 1;
	}

	memmove((void *)((char *)(vec->data) + vec->size * vec->elem_s), data,
		vec->elem_s);
	vec->size++;

	return 0;
}

void *pop_back(struct vector *vec)
{
	void *elem = NULL;

	if (vec->size > 0) {
		void *e = (void *)((char *)(vec->data) +
				   --vec->size * vec->elem_s);
		elem = malloc(vec->elem_s);

		memmove(elem, e, vec->elem_s);
		memset(e, 0x0, vec->elem_s);
	}

	return elem;
}

void *at(struct vector *vec, size_t i)
{
	if (i >= vec->size) {
		return NULL;
	}

	return (void *)((char *)(vec->data) + i * vec->elem_s);
}

void reserve(struct vector *vec, size_t ncap)
{
	if (ncap <= vec->capacity)
		return;

	vec->capacity = ncap;
	void *tmp = realloc(vec->data, vec->elem_s * vec->capacity);
	if (tmp == NULL) {
		free_data(vec->data);
		free(vec->data);
		vec->data = NULL;
	} else {
		vec->data = tmp;
	}
}

void clear(struct vector *vec)
{
	free_data(vec);
	memset(vec->data, 0x0, vec->elem_s * vec->size);

	vec->size = 0;
}

void *front(struct vector *vec)
{
	return vec->data;
}

void *back(struct vector *vec)
{
	if (!vec->size) {
		return NULL;
	}

	return (void *)((char *)vec->data + (vec->size - 1) * vec->elem_s);
}
