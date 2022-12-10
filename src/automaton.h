#ifndef SLASH_AUTOMATON_H
#define SLASH_AUTOMATON_H
#include "vector.h"

struct automaton;
struct automaton *make_automaton(struct vector *regex);
int check_regex(struct automaton *a, char *s);
void free_automaton(struct automaton *a);

#endif
