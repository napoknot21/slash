#include "automaton.h"

#include "token.h"
#include "vector.h"

#include <string.h>

struct link;

struct state {
	struct vector *links;
	int end;
	int start;
};

struct link {
	struct state *in;
	struct state *out;
	struct string *keys; //(start, end) with ASCII code for intervals such
			     // as [a-e];
};

struct automaton {
	struct vector *states;
};

struct p_state {
	struct state *s;
	int ind;
};

static struct link *make_link(struct token *tok, struct state *current,
			      struct state *next);
static struct state *make_state(int end, int start);
static void u_free_link(void *ln);
static void u_free_state(void *s);
static int check_link(struct link *ln, char c);
static struct state *check_state(struct state *state, char c,
				 struct vector *stack, int ind);
static int not_in(struct vector *links, struct link *ln);

static void u_free_link(void *ln)
{
	free_string(((struct link *)ln)->keys);
}

static struct state *make_state(int end, int start)
{
	struct state *s = malloc(sizeof(struct state));
	if (s == NULL)
		return NULL;
	s->links = make_vector(sizeof(struct vector), u_free_link);
	if (s->links == NULL) {
		free(s);
		return NULL;
	}
	s->end = end;
	s->start = start;
	return s;
}

static void u_free_state(void *s)
{
	free_vector(((struct state *)s)->links);
}

static struct link *make_link(struct token *tok, struct state *current,
			      struct state *next)
{
	struct link *ln = malloc(sizeof(struct link));
	if (ln == NULL)
		return NULL;
	ln->keys = make_string("");
	if (ln->keys == NULL) {
		free(ln);
		return NULL;
	}
	append(ln->keys, tok->data);
	ln->in = current;
	ln->out = next;
	if (ln->out == NULL) {
		free_string(ln->keys);
		free(ln);
		return NULL;
	}
	return ln;
}

static int not_in(struct vector *links, struct link *ln)
{
	for (size_t i = 0; i < links->size; i++) {
		struct link *tmp = at(links, i);
		if (cmp_str(tmp->keys, ln->keys))
			return 0;
	}
	return 1;
}

static int check_link(struct link *ln, char c)
{
	if (ln->in == ln->out)
		return 1;
	for (size_t i = 0; i < size_str(ln->keys); i += 2) {
		char start = *at_str(ln->keys, i);
		char end = *at_str(ln->keys, i + 1);
		if (start <= c && c <= end)
			return 1;
	}
	return 0;
}

static struct state *check_state(struct state *state, char c,
				 struct vector *stack, int ind)
{
	int cnt = 0;
	for (size_t i = 0; i < state->links->size; i++) {
		struct link *ln = at(state->links, i);
		if (check_link(ln, c)) {
			cnt++;
			struct p_state tmp = {.s = ln->out, .ind = ind};
			push_back(stack, &tmp);
		}
	}
	if (cnt != 0) {
		struct p_state *p_ret = at(stack, stack->size - 1);
		struct state *ret = p_ret->s;
		pop_back(stack);
		return ret;
	}
	return NULL;
}

struct automaton *make_automaton(struct vector *regex)
{
	struct automaton *a = malloc(sizeof(struct automaton));
	if (a == NULL)
		return NULL;
	a->states = make_vector(sizeof(struct state), u_free_state);
	struct state *current = make_state(0, 1);
	push_back(a->states, current);
	for (size_t i = 0; i < regex->size; i++) {
		struct token *tok = at(regex, i);
		struct state *next =
			(tok->type_spec == STAR) ? make_state(0, 0) : current;
		struct link *ln = make_link(tok, current, next);
		if (ln == NULL) {
			goto error_malloc;
		}
		if (not_in(current->links, ln))
			push_back(current->links, ln);
		if (current != next) {
			current = next;
			push_back(a->states, next);
		}
		free(ln);
	}
	current->end = 1;
	return a;
error_malloc:
	if (a != NULL)
		free_automaton(a);
	return NULL;
}

int check_regex(struct automaton *a, char *s)
{
	if (a == NULL || s == NULL)
		return 0;
	struct vector *stack = make_vector(sizeof(struct p_state), NULL);
	struct state *current = at(a->states, 0);
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		current = check_state(current, s[i], stack, i);
		if (current == NULL) {
			if (stack->size == 0) {
				free_vector(stack);
				return 0;
			} else {
				struct p_state *tmp = at(stack, stack->size -1);
				i = tmp->ind;
				current = tmp->s;
			}
		}
	}
	return current->end;
}

void free_automaton(struct automaton *a)
{
	if (a == NULL)
		return;
	free_vector(a->states);
	free(a);
}
