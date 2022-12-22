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
	if (value != NULL) {
		new->path = malloc(sizeof(struct string));
		copy_str(value, new->path);
	} else {
		new->path = NULL;
	}
	if (subtokens != NULL) {
		new->subtokens = malloc(sizeof(struct vector));
		copy_vec(subtokens, new->subtokens);
	} else {
		new->subtokens = NULL;
	}
	new->is_dir = is_dir;
	return new;
}

static void destruct_jt(void *jt)
{
	struct joker_token *del = (struct joker_token *)jt;
	if (del->path != NULL)
		free_string(del->path);
	if (del->subtokens != NULL)
		free_vector(del->subtokens);
}

static void copy_jt(struct joker_token *jt, struct joker_token *cp)
{
	if (jt->path != NULL) {
		cp->path = malloc(sizeof(struct string));
		copy_str(jt->path, cp->path);
	} else {
		cp->path = NULL;
	}
	if (jt->subtokens != NULL) {
		cp->subtokens = malloc(sizeof(struct vector));
		copy_vec(jt->subtokens, cp->subtokens);
	} else {
		cp->subtokens = NULL;
	}
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

static struct joker_token *make_star(int is_dir)
{
	struct string *s = make_string("*");
	struct vector *subtoks = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token,
		(void (*)(void *, void *))copy_token);
	struct token *tok = make_token("*", JOKER, STAR);
	push_back(subtoks, tok);
	struct joker_token *result = make_jt(s, subtoks, REGEX, is_dir);
	free_vector(subtoks);
	free_string(s);
	free_token(tok);
	return result;
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
	int has_bracket = 0;
	struct token *last = NULL;
	int is_none = 0;

	for (size_t i = 0; i < len; i++) {
		enum token_type_spec spec = compute_token_type(arg[i]);
		int c = 0;
		switch (spec) {
		case HYPHEN:
			if (spec == HYPHEN && !has_bracket) {
				spec = SPEC_NONE;
			} else {
				c = 0;
				break;
			}
		// fall through
		case SPEC_NONE:
			is_none = 1;
			c = 1;
			break;
		case RBRACKET:
			c = 0;
			has_bracket = 0;
			break;
		case LBRACKET:
			c = 0;
			has_bracket = 1;
			break;
		default:
			c = 0;
		}
		if (c)
			continue;
		if (last != NULL) {
			push_back(tokens, last);
			free(last);
			last = NULL;
		}
		if (is_none) {
			struct token *tmp =
				compute_tok(arg, start, i - 1, SPEC_NONE);
			push_back(tokens, tmp);
			free(tmp);
			start = i;
		}
		last = compute_tok(arg, start, i, spec);
		start = i + 1;
		is_none = 0;
	}
	if (last != NULL) {
		push_back(tokens, last);
		free(last);
	}
	if (is_none) {
		struct token *tmp = compute_tok(arg, start, len, SPEC_NONE);
		push_back(tokens, tmp);
		free(tmp);
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

static enum token_type_spec compute_jt_type(struct string *buf,
					    struct vector *subtokens)
{
	if (subtokens->size != 0 &&
	    ((struct token *)at(subtokens, 0))->type_spec == DSTAR)
		return DSTAR;
	else
		return has_joker(buf) ? REGEX : PATH;
}

static struct joker_token *add_token(struct string *buf,
				     struct joker_token *last,
				     struct vector *subtokens)
{
	struct joker_token *jt = NULL;
	enum token_type_spec type = compute_jt_type(buf, subtokens);

	if (last != NULL && subtokens->size == 1 && last->type == PATH &&
	    type == PATH) {
		push_back_str(last->path, '/');
		append(last->path, buf);
		clear(subtokens);
		clear_str(buf);
	} else {
		if (last == NULL && size_str(buf) == 0) {
			push_back_str(buf, '/');
			return NULL;
		}
		if (has_bracket)
			subtokens = expand_bracket(subtokens);
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
	case DSTAR:
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
	enum token_type_spec type = compute_jt_type(buf, subtoks);
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
	size_t i = 0;
	struct token *tok = at(tokens, 0);
	if (tok->type_spec == SLASH) {
		i++;
		struct joker_token *jt = make_jt(NULL, NULL, SLASH, 1);
		push_back(result, jt);
		free_jt(jt);
	}
	for (; i < tokens->size && error == 0; i++) {
		tok = at(tokens, i);
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

static int search(struct string *path, struct vector *entries,
		  struct vector *result, struct vector *jtokens,
		  struct joker_token *jt, size_t i, int incr, int link)
{
	struct stat st, lst;
	char *c_path = NULL;
	char *name = NULL;
	for (size_t j = 0; j < entries->size; j++) {
		name = c_str(at(entries, j));
		if (size_str(path) + strlen(name) + 2 >= PATH_MAX) {
			free(name);
			continue;
		}
		append_cstr(path, name);

		c_path = c_str(path);
		if (stat(c_path, &st) == -1 || lstat(c_path, &lst)) {
			goto error;
		}
		if ((jtokens->size == i + (size_t)incr) &&
		    ((!jt->is_dir) || (jt->is_dir && S_ISDIR(st.st_mode)))) {
			struct token *tok = make_token(c_path, ARG, SPEC_NONE);
			push_back(result, tok);
			// free(tok->data);
			free(tok);
		}
		free(c_path);
		c_path = NULL;
		if (S_ISDIR(st.st_mode) && jt->is_dir &&
		    (link || (!link && !S_ISLNK(lst.st_mode)))) {
			push_back_str(path, '/');
			compute_pattern(jtokens, result, path,
					(i + (size_t)incr));
			pop_back_str(path);
		}
		// truncate
		truncate_str(path, strlen(name));
		free(name);
		name = NULL;
	}
	return 0;
error:
	if (name)
		free(name);
	if (c_path)
		free(c_path);
	return -1;
}

static int is_hidden(char *name)
{
	return name[0] == '.';
}

static void analyze_dir(DIR *dir, struct automaton *a, struct vector *entries,
			struct vector *dstar, int is_dstar)
{
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		struct string *s_name = make_string(entry->d_name);
		if (is_dstar && !is_hidden(entry->d_name))
			push_back(dstar, s_name);
		if (check_regex(a, entry->d_name)) {
			push_back(entries, s_name);
		}
		free_string(s_name);
	}
}

static int compute_pattern(struct vector *jtokens, struct vector *result,
			   struct string *path, size_t i)
{
	struct vector *entries = NULL;
	struct vector *dstar = NULL;
	struct automaton *automaton = NULL;
	char *c_path = NULL;
	DIR *dir = NULL;
	int is_dstar = 0;
	struct stat st;
	if (jtokens->size == i) {
		return 0;
	}
	struct joker_token *jt = at(jtokens, i);
	size_t n_path = 0;

	if (jt->type == PATH) {
		append(path, jt->path);
		n_path = size_str(jt->path);
		i++;
		if (jtokens->size <= i) {
			c_path = c_str(path);
			if (stat(c_path, &st) == 0)
				add_path(jt, result, path); // check exist
			free(c_path);
			truncate_str(path, n_path);
			return 0;
		}
		push_back_str(path, '/');
		n_path++;
		jt = at(jtokens, i);
	}

	if (jt->type == DSTAR) {
		i++;
		if (i >= jtokens->size) {
			struct joker_token *tmp = make_star(jt->is_dir);
			push_back(jtokens, tmp);
			free_jt(tmp);
			jt->is_dir = 1;
		}
		jt = at(jtokens, i);
		is_dstar = 1;
	}

	if (size_str(path) == 0) {
		dir = opendir(".");
	} else {
		c_path = c_str(path);
		dir = opendir(c_path);
		free(c_path);
		c_path = NULL;
	}
	if ((dir) == NULL)
		goto error;
	entries = make_vector(sizeof(struct string),
			      (void (*)(void *))destruct_string,
			      (void (*)(void *, void *))copy_str);
	dstar = make_vector(sizeof(struct string),
			    (void (*)(void *))destruct_string,
			    (void (*)(void *, void *))copy_str);
	automaton = make_automaton(jt->subtokens);
	analyze_dir(dir, automaton, entries, dstar, is_dstar);
	free_automaton(automaton);
	automaton = NULL;
	closedir(dir);
	dir = NULL;
	if (entries->size != 0) // Recursive call
		search(path, entries, result, jtokens, jt, i, 1, 1);
	if (is_dstar && dstar->size != 0) { // Recursive call
		jt = at(jtokens, i - 1);
		search(path, dstar, result, jtokens, jt, i - 1, 0, 0);
	}
	free_vector(entries);
	free_vector(dstar);
	truncate_str(path, n_path);
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
	truncate_str(path, n_path);
	return -1;
}

static int interpret(struct vector *jtokens, struct vector *result)
{
	struct string *path = make_string("");
	if (path == NULL)
		return -1;
	struct joker_token *jt = at(jtokens, 0);
	size_t i = 0;
	if (jt->type == SLASH) {
		i++;
		push_back_str(path, '/');
	}
	int ret = compute_pattern(jtokens, result, path, i);
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
