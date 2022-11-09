#include "vector.h"
#include <string.h>

vector * make_vector(size_t elem_s)
{
	vector * vec = malloc(sizeof(struct vector));
	
	vec->capacity = VECTOR_DEFAULT_CAPACITY;
	vec->size = 0;
	vec->elem_s = elem_s;

	vec->data = malloc(elem_s * vec->capacity);

	return vec;

}

void free_vector(vector * vec)
{
	free(vec->data);
	free(vec);
}

int push_back(vector * vec, void * data)
{
	if(vec->size >= vec->capacity)
	{
		vec->capacity *= 2;
		vec->data = realloc(vec->data, vec->elem_s * vec->capacity);

		if(!vec->data) return 1;
	}

	memmove((void*)((char*)(vec->data) + vec->size * vec->elem_s), data, vec->elem_s);
	vec->size++;

	return 0;
}

void * pop_back(vector * vec)
{
	void * elem = NULL;

	if(vec->size > 0)
	{
		void * e = (void *)((char *)(vec->data) + --vec->size * vec->elem_s);
		elem = malloc(vec->elem_s);

		memmove(elem, e, vec->elem_s);
		memset(e, 0x0, vec->elem_s);
	}

	return elem;
}

void * at(vector * vec, size_t i)
{
	if(i >= vec->size) 
	{
		return NULL;
	}
	
	return (void*)((char*)(vec->data) + i * vec->elem_s);
}

void clear(vector * vec)
{
	memset(vec->data, 0x0, vec->elem_s * vec->size);
}
