#include "joker.h"

#include "automaton.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int has_hyphen = 0;
static int has_bracket = 0;
static int error = 0;

#define ESYNTAX 1
#define EUNKNOWN 2

struct joker_token {
	struct string *path;
	struct vector *subtokens;
	enum token_type_spec type;
	int is_dir;
};

static struct joker_token *make_jt(struct string *value,
				   struct vector *subtokens,
				   enum token_type_spec type, int is_dir);
static void destruct_jt(void *jt);
static void free_jt(struct joker_token *jt);
static void copy_jt(struct joker_token *jt, struct joker_token *cp);
static struct vector *lex(struct token *t);
static struct vector *build_tokens(char *arg);
static enum token_type_spec compute_token_type(char c);
static int push_back_vec(struct vector *u, struct vector *v);
static struct token *make_slash();
static struct vector *parse(struct vector *tokens);
static struct joker_token *add_token(struct string *s, struct joker_token *last,
				     struct vector *subtokens);
static struct vector *expand_bracket(struct vector *subtokens);
static int compute_pattern(struct vector *jtokens, struct vector *result,
			   struct string *path, size_t i);
static int has_joker(struct string *s);

//###################################################
static struct joker_token *make_jt(struct string *value,
				   struct vector *subtokens,
				   enum token_type_spec type, int is_dir)

{
	struct joker_token *new = malloc(sizeof(struct joker_token));
	if (new == NULL)
		return NULL;
	new->type = type;
	new->path = malloc(sizeof(struct string));
	copy_str(value, new->path);
	new->subtokens = malloc(sizeof(struct vector));
	copy_vec(subtokens, new->subtokens);
	new->is_dir = is_dir;
	return new;
}

static void destruct_jt(void *jt)
{
	struct joker_token *del = (struct joker_token *)jt;
	free_string(del->path);
	if (del->subtokens != NULL)
		free_vector(del->subtokens);
}

static void copy_jt(struct joker_token *jt, struct joker_token *cp)
{
	cp->path = malloc(sizeof(struct string));
	copy_str(jt->path, cp->path);
	cp->subtokens = malloc(sizeof(struct vector));
	copy_vec(jt->subtokens, cp->subtokens);
	cp->type = jt->type;
	cp->is_dir = jt->is_dir;
}

static void free_jt(struct joker_token *jt)
{
	destruct_jt(jt);
	free(jt);
}

static struct token *make_slash()
{
	return make_token("/", JOKER, SLASH);
}

static int push_back_vec(struct vector *u, struct vector *v)
{
	for (size_t i = 0; i < v->size; i++) {
		if (push_back(u, at(v, i)) != 0)
			return 1;
	}
	return 0;
}

static enum token_type_spec compute_token_type(char c)
{
	switch (c) {
	case '*':
		return STAR;
	case '[':
		return LBRACKET;
	case ']':
		return RBRACKET;
	case '-':
		return HYPHEN;
	default:
		return SPEC_NONE;
		break;
	}
}

static struct token *compute_tok(char *arg, int start, int end, int spec)
{
	end = (start != end) ? end : start + 1;
	char *buf = malloc(end - start + 1);
	memcpy(buf, arg + start, end - start);
	buf[end - start] = '\0';
	struct token *t = make_token(buf, JOKER, spec);
	free(buf);
	return t;
}

static struct vector *build_tokens(char *arg)
{
	size_t len = strlen(arg);
	struct vector *tokens = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token, NULL);
	if (tokens == NULL)
		return NULL;
	if (len == 2 && arg[0] == '*' && arg[1] == '*') {
		struct token *t = make_token(arg, JOKER, DSTAR);
		push_back(tokens, t);
		free(t);
		return tokens;
	}

	size_t start = 0;
	size_t end = 0;
	int has_bracket = 0;
	enum token_type_spec last = INTERNAL;

	for (size_t i = 0; i < len; i++) {
		enum token_type_spec spec = compute_token_type(arg[i]);
		if (last == INTERNAL)
			last = spec;
		if (spec == LBRACKET)
			has_bracket = 1;
		if (spec == RBRACKET)
			has_bracket = 0;
		if (last == spec && spec == SPEC_NONE && !has_bracket) {
			end++;
		} else {
			struct token *t = compute_tok(arg, start, end, last);
			start = end;
			end++;
			push_back(tokens, t);
			free(t);
			last = spec;
		}
	}
	if (start < len) {
		struct token *t = compute_tok(arg, start, end, last);
		push_back(tokens, t);
		free(t);
	}
	return tokens;
}

static struct vector *lex(struct token *t)
{
	char *line = NULL;
	struct token *slash = make_slash();
	struct vector *tokens =
		make_vector(sizeof(*t), (void (*)(void *))destruct_token,
			    (void (*)(void *, void *))copy_token);
	if (slash == NULL || tokens == NULL)
		goto error_null;
	line = c_str(t->data);
	int is_dir = (*at_str(t->data, size_str(t->data) - 1) == '/');
	if (line == NULL)
		goto error_null;
	if (*line == '/') {
		struct token *t = make_slash();
		push_back(tokens, t);
		free_token(t);
	}
	char *delimiters = "/";
	char *s = strtok(line, delimiters);
	while (s != NULL) {
		struct vector *tmp = build_tokens(s);
		if (tmp == NULL)
			goto error_null;
		push_back_vec(tokens, tmp);
		push_back(tokens, slash);
		s = strtok(NULL, delimiters);
		free_vector(tmp);
	}
	if (!is_dir)
		pop_back(tokens);
	free(line);
	free_token(slash);
	return tokens;

error_null:
	if (tokens != NULL)
		free_vector(tokens);
	if (line != NULL)
		free(line);
	if (slash != NULL)
		free_token(slash);
	return NULL;
}

static struct joker_token *add_token(struct string *buf,
				     struct joker_token *last,
				     struct vector *subtokens)
{
	struct joker_token *jt = NULL;
	if (last != NULL && subtokens->size == 1 && last->type == PATH) {
		push_back_str(last->path, '/');
		append(last->path, buf);
		clear(subtokens);
		clear_str(buf);
	} else {
		if (last == NULL && size_str(buf) == 0)
			push_back_str(buf, '/');
		if (has_bracket)
			subtokens = expand_bracket(subtokens);
		enum token_type_spec type = has_joker(buf) ? REGEX : PATH;
		jt = make_jt(buf, subtokens, type, 1);
	}
	return jt;
}

static struct joker_token *compute_joker_tok(struct token *tok,
					     struct string *buffer,
					     struct joker_token *last,
					     struct vector *subtokens)
{
	switch (tok->type_spec) { // Faire directement les changement ici de
				  // manière à que seulment add_token s'occupe
				  // de cas limites ( SLASH )
	case SLASH:
		return add_token(buffer, last, subtokens);
	case SPEC_NONE:
		has_hyphen = 0;
		if (has_bracket) {
			tok->type_spec = SPEC_NONE;
			struct token *some = at(subtokens, subtokens->size - 1);
			if (has_hyphen) {
				char *tmp = at_str(some->data,
						   size_str(some->data) - 1);
				*tmp = *at_str(tok->data, 0);
			} else {
				append(some->data, tok->data);
				append(some->data, tok->data);
			}
			break;
		}
		append(buffer, tok->data);
		push_back(subtokens, tok);
		break;
	case STAR:
		if (has_bracket) {
			tok->type_spec = SPEC_NONE;
			break;
		}
		append(buffer, tok->data);
		push_back(subtokens, tok);
		break;

	case HYPHEN:
		has_hyphen = 1;
		append(buffer, tok->data);
		break;

	case LBRACKET:
		has_bracket = 1;
		struct token *some = make_token("", JOKER, SOME);
		append(buffer, tok->data);
		push_back(subtokens, some);
		free_token(some);
		break;
	case RBRACKET:
		has_bracket = 0;
		break;

	default:
		error = EUNKNOWN;
	}

	return NULL;
}

static void add_last_token(struct vector *result, struct string *buf,
			   struct vector *subtoks)
{
	if (size_str(buf) == 0) {
		return;
	}
	enum token_type_spec type = has_joker(buf) ? REGEX : PATH;
	struct joker_token *tmp = make_jt(buf, subtoks, type, 0);
	push_back(result, tmp);
	free_jt(tmp);
	clear(subtoks);
	clear_str(buf);
}

static struct vector *parse(struct vector *tokens)
{
	struct vector *subtoks = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token,
		(void (*)(void *, void *))copy_token);
	struct joker_token *last = NULL;
	struct string *buf = make_string("");
	struct vector *result =
		make_vector(sizeof(struct joker_token), destruct_jt,
			    (void (*)(void *, void *))copy_jt);
	for (size_t i = 0; i < tokens->size && error == 0; i++) {
		struct token *tok = at(tokens, i);
		struct joker_token *tmp =
			compute_joker_tok(tok, buf, last, subtoks);
		if (tmp != NULL) {
			push_back(result, tmp);
			clear(subtoks);
			clear_str(buf);
			free_jt(tmp);
			last = at(result, result->size - 1);
		}
	}
	add_last_token(result, buf, subtoks);
	free_string(buf);
	free_vector(subtoks);
	if (has_hyphen || has_bracket) {
		result->free = destruct_jt;
		free_vector(result);
		return NULL;
	}
	return result;
}

static struct vector *expand_bracket(struct vector *subtokens)
{ // TODO
	// struct token *regex = make_token()
	return subtokens;
}

static int has_joker(struct string *s)
{
	for (size_t i = 0; i < s->cnt->size; i++) {
		if (compute_token_type(*at_str(s, i)) != SPEC_NONE) {
			return 1;
		}
	}
	return 0;
}

static void truncate_str(struct string *s, size_t len)
{
	for (size_t i = 0; i < len; i++)
		pop_back_str(s);
}

static void add_path(struct joker_token *jt, struct vector *result,
		     struct string *path)
{
	struct stat st;
	char *c_path = c_str(path);
	if (size_str(path) > PATH_MAX || stat(c_path, &st) ||
	    (jt->is_dir && S_ISDIR(st.st_mode))) {
		free(c_path);
		return;
	}
	struct token *t = make_token(c_path, ARG, SPEC_NONE);
	push_back(result, t);
	free(t);
	free(c_path);
}

static int compute_pattern(struct vector *jtokens, struct vector *result,
			   struct string *path, size_t i)
{
	struct vector *entries = NULL;
	struct automaton *automaton = NULL;
	DIR *dir = NULL;
	if (jtokens->size == i) {
		return 0;
	}
	struct joker_token *jt = at(jtokens, i);

	if (jt->type == PATH) {
		append(path, jt->path);
		i++;
		if (jtokens->size <= i) {
			add_path(jt, result, path);
			return 0;
		}
		jt = at(jtokens, i);
	}
	char *c_path = c_str(path);
	if ((dir = opendir(c_path)) == NULL)
		goto error;
	free(c_path);
	c_path = NULL;
	entries = make_vector(sizeof(struct string),
			      (void (*)(void *))destruct_string, NULL);
	automaton = make_automaton(jt->subtokens);
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (check_regex(automaton, entry->d_name)) {
			struct string *s_name = make_string(entry->d_name);
			push_back(entries, s_name);
			free(s_name);
		}
	}
	free_automaton(automaton);
	automaton = NULL;
	closedir(dir);
	dir = NULL;
	struct stat st;
	size_t j = 0;
	for (j = 0; j < entries->size; j++) {
		char *name = c_str(at(entries, j));
		if (size_str(path) + strlen(name) + 2 >= PATH_MAX) {
			free(name);
			continue;
		}
		append_cstr(path, name);

		c_path = c_str(path);
		if (stat(c_path, &st) == -1) {
			goto error;
		}
		if ((jtokens->size == i + 1) &&
		    ((!jt->is_dir) || (jt->is_dir && S_ISDIR(st.st_mode)))) {
			struct token *tok = make_token(c_path, ARG, SPEC_NONE);
			push_back(result, tok);
			// free(tok->data);
			free(tok);
		}
		if (S_ISDIR(st.st_mode) && jt->is_dir)
			compute_pattern(jtokens, result, path, (i + 1));
		// truncate
		truncate_str(path, strlen(name));
		free(c_path);
		c_path = NULL;
		free(name);
	}
	free_vector(entries);
	return 0;

error:
	if (c_path != NULL)
		free(c_path);
	if (entries != NULL) {
		free_vector(entries);
	}
	if (dir != NULL)
		closedir(dir);
	if (automaton != NULL)
		free_automaton(automaton);

	return -1;
}

static int interpret(struct vector *jtokens, struct vector *result)
{
	struct joker_token *jt = (struct joker_token *)at(jtokens, 0);
	char *init;
	int start;
	if ((jt->type == PATH) && (*at_str(jt->path, 0) == '/')) {
		start = 1;
		init = "";
	} else {
		start = 0;
		init = getenv("PWD"); // FIXME: retirer quand cd sera fix
	}
	struct string *path = make_string(init);
	push_back_str(path, '/');
	if (path == NULL)
		return -1;
	int ret = compute_pattern(jtokens, result, path, start);
	if (path != NULL)
		free_string(path);
	return ret;
}

struct vector *expand_jokers(struct token *tok)
{
	has_bracket = 0;
	has_hyphen = 0;
	if (!has_joker(tok->data))
		return NULL;

	struct vector *l = lex(tok);
	if (l == NULL)
		return NULL;
	struct vector *p = parse(l);
	free_vector(l);
	if (p == NULL)
		return NULL;
	struct vector *result = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token, NULL);
	interpret(p, result);
	free_vector(p);
	if (result->size == 0) {
		free_vector(result);
		return NULL;
	}
	return result;
}
