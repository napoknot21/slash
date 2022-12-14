#include "automaton.h"

#include "token.h"
#include "vector.h"

#include <string.h>

struct link;

static int n_checked = 0;

struct state {
	struct vector *links;
	int end;
	int start;
};

struct link {
	int in;
	int out;
	struct string *keys; //(start, end) with ASCII code for intervals such
			     // as [a-e];
	enum token_type_spec type;
};

struct automaton {
	struct vector *states;
};

struct p_state {
	int s;
	int ind;
};

static struct link *make_link(struct token *tok, int current, int next);
static struct state *make_state(int end, int start);
static void destruct_link(void *ln);
static void destruct_state(void *s);
static void copy_link(struct link *ln, struct link *cp);
static int check_link(struct link *ln, char *c);
static struct state *check_state(struct automaton *a, struct state *state,
				 char *c, struct vector *stack, int ind);
static int not_in(struct vector *links, struct link *ln);
static int check_some(struct string *keys, char c);
static int check_none(struct string *keys, char *c);

static void destruct_link(void *ln)
{
	free_string(((struct link *)ln)->keys);
}

static struct state *make_state(int end, int start)
{
	struct state *s = malloc(sizeof(struct state));
	if (s == NULL)
		return NULL;
	s->links = make_vector(sizeof(struct vector), destruct_link,
			       (void (*)(void *, void *))copy_link);
	if (s->links == NULL) {
		free(s);
		return NULL;
	}
	s->end = end;
	s->start = start;
	return s;
}

static void destruct_state(void *s)
{
	free_vector(((struct state *)s)->links);
}

static struct link *make_link(struct token *tok, int current, int next)
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
	ln->type = tok->type_spec;
	return ln;
}

static void copy_link(struct link *ln, struct link *cp)
{
	cp->keys = malloc(sizeof(struct string));
	copy_str(ln->keys, cp->keys);
	cp->out = ln->out;
	cp->in = ln->in;
	cp->type = ln->type;
}

static int not_in(struct vector *links, struct link *ln)
{
	for (size_t i = 0; i < links->size; i++) {
		struct link *tmp = at(links, i);
		if (cmp_str(tmp->keys, ln->keys) == 0)
			return 0;
	}
	return 1;
}
static int check_some(struct string *keys, char c)
{
	for (size_t i = 0; i < size_str(keys); i += 2) {
		char start = *at_str(keys, i);
		char end = *at_str(keys, i + 1);
		if (start <= c && c <= end)
			return 1;
	}
	return 0;
}

static int check_none(struct string *keys, char *c)
{
	size_t len = strlen(c);
	size_t size = size_str(keys);
	if (len < size)
		return 0;
	char tmp[size + 1];
	memcpy(tmp, c, size);
	tmp[size] = '\0';
	char *key = c_str(keys);
	int ret = strcmp(key, tmp);
	free(key);
	if (ret == 0) {
		n_checked = size;
		return 1;
	}
	return 0;
}
static int check_link(struct link *ln, char *c)
{
	n_checked = 1;
	switch (ln->type) {
	case SOME:
		return check_some(ln->keys, *c);
	case SPEC_NONE:
		return check_none(ln->keys, c);
	case STAR:
		return 1;
	default:
		return 0;
	}
}

static struct state *check_state(struct automaton *a, struct state *state,
				 char *c, struct vector *stack, int ind)
{
	int cnt = 0;
	for (size_t i = 0; i < state->links->size; i++) {
		struct link *ln = at(state->links, i);
		if (check_link(ln, c)) {
			cnt++;
			struct p_state tmp = {.s = ln->out,
					      .ind = ind + n_checked - 1};
			push_back(stack, &tmp);
		}
	}
	if (cnt != 0) {
		struct p_state *p_ret = at(stack, stack->size - 1);
		struct state *ret = at(a->states, p_ret->s);
		pop_back(stack);
		return ret;
	}
	return NULL;
}

static int check_hidden(struct state *state, char *name)
{
	if (name[0] != '.')
		return 1;
	for (size_t i = 0; i < state->links->size; i++) {
		struct link *ln = at(state->links, i);
		if (ln->type == STAR)
			return 0;
	}
	return 1;
}

struct automaton *make_automaton(struct vector *regex)
{
	struct automaton *a = malloc(sizeof(struct automaton));
	if (a == NULL)
		return NULL;
	a->states = make_vector(sizeof(struct state), destruct_state, NULL);
	struct state *tmp = make_state(0, 1);
	struct state *next = NULL;
	push_back(a->states, tmp);
	free(tmp);
	int current = 0;
	for (size_t i = 0; i < regex->size; i++) {
		struct token *tok = at(regex, i);
		int next = (tok->type_spec != STAR) ? current + 1 : current;
		struct link *ln = make_link(tok, current, next);
		if (ln == NULL) {
			goto error_malloc;
		}
		struct state *c_state = at(a->states, current);
		if (not_in(c_state->links, ln))
			push_back(c_state->links, ln);
		if (current != next) {
			tmp = make_state(0, 0);
			push_back(a->states, tmp);
			free(tmp);
			current = next;
		}
		destruct_link(ln);
		free(ln);
	}
	struct state *c_state = at(a->states, current);
	c_state->end = 1;
	return a;
error_malloc:
	if (a != NULL)
		free_automaton(a);
	return NULL;
}

int check_regex(struct automaton *a, char *s)
{
	n_checked = 0;
	if (a == NULL || s == NULL)
		return 0;
	struct vector *stack = make_vector(sizeof(struct p_state), NULL, NULL);
	struct state *current = at(a->states, 0);
	if (!check_hidden(current, s)) {
		free_vector(stack);
		return 0;
	}
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		current = check_state(a, current, s + i, stack, i);
		i += n_checked - 1;
		if (current == NULL) {
			if (stack->size == 0) {
				free_vector(stack);
				return 0;
			} else {
				struct p_state *tmp =
					at(stack, stack->size - 1);
				i = tmp->ind;
				current = at(a->states, tmp->s);
			}
		}
	}
	free_vector(stack);
	return current->end;
}

void free_automaton(struct automaton *a)
{
	if (a == NULL)
		return;
	free_vector(a->states);
	free(a);
}
