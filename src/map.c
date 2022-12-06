#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "map.h"

/*
 * Allocation d'une table vide
 */

map * make_map(size_t elem_size)
{
	map * rmap = malloc(sizeof(map));

	rmap->capacity = DEFAULT_MAP_SIZE;
	rmap->size = 0;
	rmap->data = malloc(sizeof(map_element) * rmap->capacity);

	rmap->elem_size = elem_size;

	for(size_t i = 0; i < rmap->capacity; i++)
	{
		rmap->data[i].status = NONE;
		rmap->data[i].keyhash = -1;
		rmap->data[i].val = NULL;
	}

	rmap->alpha = 0.8;
	rmap->beta = 0.5;

	return rmap;
}

/*
 * Désallocation de la table
 */

void free_map(map * m)
{
	free(m->data);
	free(m);
}

int hash_data(void * d, size_t size)
{
	int hashsum = 0;

	for(size_t b = 0; b < size; b++)
	{
		char buffer;

		memmove(&buffer, (void*)((char *) d + b), HASH_DATA_OFFSET);
		hashsum += (1 + b) * (int) buffer;		
	}
	
	return hashsum;
}

int f_hash(int k, size_t cap)
{
	return (3 * k) % cap;
}

int d_hash(int k, size_t cap)
{	
	return ((5 * k) + 1) % cap;
}

void resize(map * m)
{		
	map_element * nd = m->data;	

	m->capacity *= 2;
	m->size = 0;

	m->data = malloc(m->capacity * sizeof(map_element));

	for(size_t i = 0; i < m->capacity; i++)
	{
		m->data[i].status = NONE;
		m->data[i].keyhash = -1;
		m->data[i].val = NULL;	
	}

	for(size_t j = 0; j < m->capacity / 2; j++)
	{
		if(nd[j].status != FILLED)
		{
			continue;
		}	
	
		map_insert(nd[j].keyhash, nd[j].val, m);
	}

	free(nd);	

}

int map_insert(int key, void * val, map * m)
{
	if(m->size >= (size_t)(m->alpha * m->capacity))
	{	
		resize(m);
	}

	int ptr = f_hash(key, m->capacity);

	if(ptr == -1)
	{
		return -1;
	}
	
	map_element * current = &m->data[ptr];

	if(current->keyhash != key)
	{
		while(current->status == FILLED)
		{
			ptr = (ptr + d_hash(ptr, m->capacity)) % m->capacity;
			current = &m->data[ptr];		
		}
	}	

	m->data[ptr].status = FILLED;
	m->data[ptr].keyhash = key;
	m->data[ptr].val = malloc(m->elem_size);

	memmove(m->data[ptr].val, val, m->elem_size);
	m->size++;	

	return 0;
}

int map_remove(int key, map * m)
{
	int ptr = f_hash(key, m->capacity);
	map_element * mel = &m->data[ptr];

	int j = 0;

	while(mel->keyhash != key)
	{
		if(mel->status == NONE)
		{
			return -1;
		}

		ptr = (ptr + d_hash(ptr, m->capacity)) % m->capacity;
		mel = &m->data[ptr];

		if(++j == m->size)
		{
			return -2;
		}
	}

	mel->status = EMPTY;
	mel->keyhash = -1;
	mel->val = NULL;

	m->size--;

	return 0;
}

void * map_at(int key, map * m)
{
	int ptr = f_hash(key, m->capacity), j = 0;	

	while(key != m->data[ptr].keyhash)
	{		
		if(++j >= m->size || m->data[ptr].status == NONE)
		{
			return NULL;
		}
	
		ptr = (ptr + d_hash(ptr, m->capacity)) % m->capacity;	
	}

	return m->data[ptr].val;
}
