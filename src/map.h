#ifndef MAP_H
#define MAP_H

#define DEFAULT_MAP_SIZE 16
#define HASH_DATA_OFFSET 1
#define STOCHASTIC_SHIFT 4

enum map_element_status 
{
	EMPTY, 
	FILLED, 
	NONE
};

typedef struct map_element 
{
	enum map_element_status status;
	int keyhash;
	void * val;

} map_element;

typedef struct map
{
	size_t capacity;
	size_t size;

	map_element * data;
	size_t elem_size;

	double alpha;
	double beta;

	int (*f_hash)(int);
	int (*d_hash)(int);

} map;

/*
 * Construction/Destruction functions
 */

map * make_map(size_t);
void free_map(map * m);

/*
 * Operation functions
 */

int insert_map(map *, int, void *);
int remove_map(map *, int);
void resize_map(map *);

/*
 * Access functions
 */

void * at_map(int, map *);

/*
 * Auxiliaries
 */

int hashdata(void *, size_t);

#endif
