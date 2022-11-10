#ifndef STRING_H
#define STRING_H

#include "vector.h"

typedef struct string
{
	vector * cnt;
} string;

/*
 * Construction/Destruction functions
 */

string * make_string();
void free_string(string *);

/*
 * Operation functions
 */

void push_back_str(string *, char);
void pop_back_str(string *);
void append(string *, string *);

void clear_str(string *);

/*
 * Access function
 */

char * at_str(string *, size_t);
char * front_str(string *);
char * back_str(string *);

string * substr(string *, size_t, size_t);

#endif 
