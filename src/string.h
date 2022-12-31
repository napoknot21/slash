#ifndef STRING_H
#define STRING_H

#include "vector.h"

struct string {

	struct vector *cnt;

};

/*
 * Construction/Destruction functions
 */

struct string *make_string(const char *);
void destruct_string(struct string *);
void free_string(struct string *);
void copy_str(const struct string *, struct string *);

/*
 * Operation functions
 */

void push_back_str(struct string *, char);
void pop_back_str(struct string *);
void pop_back_n_str(struct string *str, size_t n);
void append(struct string *, struct string *);
void append_cstr(struct string *, const char *);

void clear_str(struct string *);
struct vector * split_str(struct string *, char);
struct string * bind_str(struct vector *, char);

/*
 * Access function
 */

char *at_str(struct string *, size_t);
char *front_str(struct string *);
char *back_str(struct string *);

struct string *substr(struct string *, size_t, size_t);
size_t size_str(struct string *);
int empty_str(struct string *);

int cmp_str(struct string *, struct string *);
char * c_str(struct string *);

#endif
