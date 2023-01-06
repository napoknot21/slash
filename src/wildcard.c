#include "wildcard.h"

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

struct wildcard_token {
	struct string *path;
	struct vector *subtokens;
	enum token_type_spec type;
	int is_dir;
};

struct pair {
	struct string *path;
	size_t index;
};

static struct wildcard_token *make_wt(struct string *value,
				      struct vector *subtokens,
				      enum token_type_spec type, int is_dir);
static void destruct_wt(void *wt);
static void copy_wt(struct wildcard_token *wt, struct wildcard_token *cp);
static void free_wt(struct wildcard_token *wt);
static void destruct_pair(struct pair *p);
static void copy_pair(struct pair *p, struct pair *cp);
/**
 * @brief Make a token that will represent a slash.
 * @return The slash token.
 */
static struct token *make_slash();
/**
 * @brief Make a wildcard_token that will be a star.
 * @param is_dir indicate if the files which are accepted by the regex are
 * directories.
 * @return The wildcard_token.
 */
static struct wildcard_token *make_star(int is_dir);
/**
 * @brief Compute the joker type from c
 * @param c The char that will be represented by a token
 * @return The type of the token
 */
static enum token_type_spec compute_token_type(char c);
/**
 * @brief Make a token from the arg data. Its data will be the chars from start
 * to end.
 * @param arg The string where the substring will be extracted.
 * @param start The beginning of the substring.
 * @param end The end of the sustring.
 * @param spec The token type.
 * @return A new token.
 */
static struct token *compute_tok(char *arg, int start, int end, int spec);

/**
 * @brief Builds all the token from the given string.
 * @param arg The string that will be translated.
 * @return A vector with all the computed tokens.
 */
static struct vector *build_tokens(char *arg);
/**
 * @brief Cut the token data into other tokens.
 * @param t The token that will be cut.
 * @return A vector with the computed tokens.
 */
static struct vector *lex(struct token *t);
/**
 * @brief Compute the wildcard_token type.
 * @param subtokens The wildcard_token subtokens.
 * @return the type of the wildcard_token.
 */
static enum token_type_spec compute_jt_type(struct vector *subtokens);

/**
 * @brief Compute the wildcard_token. If the last joker was a PATH, then the
 * current buffer is added to the last one and no new wildcard_token is created
 * else a new wildcard_token is created from the buffer and the vector.
 * @param buf The wildcard_token path.
 * @param last The last computed wildcard_token.
 * @param subtokens The wildcard_token subtokens.
 * @return The new wildcard_token if one is created NULL otherwise.
 */
static struct wildcard_token *add_token(struct string *buf,
					struct wildcard_token *last,
					struct vector *subtokens);
/**
 * @brief Compute the buffer for basic tokens.
 * @param tok The token wich store the data.
 * @param buffer The wildcard_token buffer.
 * @param subtokens the wildcard _token bubtokens.
 */
static void compute_spec_none(struct token *tok, struct string *buffer,

			      struct vector *subtokens);
/**
 * @brief Compute the wildcard _token from the token
 * @param tok The token wich store the data.
 * @param buffer The wildcard_token buffer.
 * @param subtokens the wildcard _token bubtokens.
 * @param last The last computed token
 */
static struct wildcard_token *compute_wildcard_tok(struct token *tok,
						   struct string *buffer,
						   struct wildcard_token *last,
						   struct vector *subtokens);
/**
 * @brief Adds the last token to the result.
 * @param result The vector which stores the computed wildcard_tokens.
 * @param buf The wildcard_token buffer.
 * @param subtoks The wildcard_token subtokens.
 */
static void add_last_token(struct vector *result, struct string *buf,
			   struct vector *subtoks);

/**
 * @brief Parses the content of the given vector
 */
static struct vector *parse(struct vector *tokens);
/**
 * @brief Check if a string has wildcard in it
 * @param s The string that will be checked.
 * @return 1 if s contrains some wildcards, 0 otherwise.
 */
static int has_wildcards(struct string *s);
/**
 * @brief Add a file path to the result if it exists.
 * @param wt The wildcard which store the regex or path.
 * @param result the expansion result.
 * @param path The file path.
 */
static void add_path(struct wildcard_token *wt, struct vector *result,
		     struct string *path);
/**
 * @brief Check if a file name is hidden.
 * @param name The filename.
 * @return 1 if the file name is hidden, 0 otherwise.
 */
static int is_hidden(char *name);
/**
 * @brief Analyze the given directorty with the given automaton.
 * @param dir the openened directory.
 * @param a The automaton that will checks the files names.
 * @param entries The vector of valid files names.
 * @param dstar The vector of valid files names for double star expansion.
 * @param is_dstar indicates if the current wildcard_token is a double star.
 */
static void analyze_dir(DIR *dir, struct automaton *a, struct vector *entries,
			struct vector *dstar, int is_dstar);
/**
 * @brief Initializes the first pair of the interpreter.
 * @param wtoken The vector of wild_cardtoken previously computed by the
 * wildcard_lexerand the wildcard_parser
 * @return The first pair
 *
 */
static struct pair init_path(struct vector *wtokens);

/**
 * @brief Analyzes the token and modifies the pair and the wtokens ccording the
 * situation.
 * @param is_dstar indicates if the current wildcard_token is a double star
 * @param p The current pair (path, index)
 * @param wtokens The vector of wildcard_tokens
 * @param The result of the wildcards expansion.
 * @return The new version of p or NULL if we reach the end of wtokens.
 */
static struct pair *analyze_tok(int *is_dstar, struct pair *p,
				struct vector *wtokens, struct vector *result);

/**
 * @brief Open a DIR according to the given path
 * @param path The dir path
 * @return The opended directory or NULL if an error occured.
 */
static DIR *opendir_path(struct string *path);

/**
 * @brief Searches the path for entries that complete the sentence represented
 * by the wtokens or directories to continue the expansion.
 * @param The current pair (path,index).
 * @param queue The pending paths queue.
 * @param entries The entries of the current directory.
 * @param result The result of the wildcards expansion.
 * @param lnk Indicates if we must follow symbolic links.
 * @param incr The incremental value for further expansion.
 * @param wtokens The vector of wildcard_tokens
 * @return The pending paths queue
 */

static int finish_path(struct pair *p, struct wildcard_token *wt,
		       struct vector *result, size_t incr, size_t len,
		       struct stat st);

static struct vector *search(struct pair *p, struct vector *queue,
			     struct vector *entries, struct vector *result,
			     int lnk, size_t incr, struct vector *wtokens);

/**
 * @brief Interprets the result computed by the parser.
 * @param wtokens The vector of computed wildcard_tokens.
 */
static struct vector *interpret(struct vector *wtokens);

struct vector *expand_wildcards(struct token *tok);

//###################################################
static struct wildcard_token *make_wt(struct string *value,
				      struct vector *subtokens,
				      enum token_type_spec type, int is_dir)

{
	struct wildcard_token *new = malloc(sizeof(struct wildcard_token));
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

static void destruct_wt(void *wt)
{
	struct wildcard_token *del = (struct wildcard_token *)wt;
	if (del->path != NULL)
		free_string(del->path);
	if (del->subtokens != NULL)
		free_vector(del->subtokens);
}

static void copy_wt(struct wildcard_token *wt, struct wildcard_token *cp)
{
	if (wt->path != NULL) {
		cp->path = malloc(sizeof(struct string));
		copy_str(wt->path, cp->path);
	} else {
		cp->path = NULL;
	}
	if (wt->subtokens != NULL) {
		cp->subtokens = malloc(sizeof(struct vector));
		copy_vec(wt->subtokens, cp->subtokens);
	} else {
		cp->subtokens = NULL;
	}
	cp->type = wt->type;
	cp->is_dir = wt->is_dir;
}

static void free_wt(struct wildcard_token *wt)
{
	destruct_wt(wt);
	free(wt);
}

static struct token *make_slash()
{
	return make_token("/", JOKER, SLASH);
}

static struct wildcard_token *make_star(int is_dir)
{
	struct string *s = make_string("*");
	struct vector *subtoks = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token,
		(void (*)(void *, void *))copy_token);
	struct token *tok = make_token("*", JOKER, STAR);
	push_back(subtoks, tok);
	struct wildcard_token *result = make_wt(s, subtoks, REGEX, is_dir);
	free_vector(subtoks);
	free_string(s);
	free_token(tok);
	return result;
}

// ======================= LEXER =========================

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
	case '?':
		return QUESTION_MARK;
	case '~':
		return TILDE;
	case '$':
		return DOLLAR;

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

static struct vector *check_special(char *arg, size_t len,
				    struct vector *tokens)
{
	enum token_type_spec spec;
	switch (arg[0]) {
	case '~':
		spec = (len == 1) ? TILDE : INTERNAL;
		break;
	case '$':
		spec = DOLLAR;
		break;
	case '*':
		spec = (len == 2 && arg[1] == '*') ? DSTAR : INTERNAL;
		break;
	default:
		spec = INTERNAL;
	}

	if (spec != INTERNAL) {
		struct token *t = make_token(arg, JOKER, spec);
		push_back(tokens, t);
		free(t);
		return tokens;
	}
	return NULL;
}

static struct vector *build_tokens(char *arg)
{
	size_t len = strlen(arg);
	struct vector *tokens = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token, NULL);

	if (tokens == NULL || len == 0)
		return NULL;
	if (check_special(arg, len, tokens) != NULL) {
		return tokens;
	}
	size_t start = 0;
	int has_bracket = 0;
	struct token *last = NULL;
	int is_none = 0;
	int is_dollar = 0;

	for (size_t i = 0; i < len; i++) {
		enum token_type_spec spec = compute_token_type(arg[i]);
		int c = 0;
		int change_bracket = 0;
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
			is_none = is_dollar ? 0 : 1;
			c = has_bracket && !is_dollar ? 0 : 1;
			break;
		case RBRACKET:
			c = 0;
			has_bracket = 0;
			break;
		case LBRACKET:
			c = 0;
			change_bracket = 1;
			break;
		case DOLLAR:
			is_dollar = 1;
			c = 1;
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
		if (is_none && !has_bracket) {
			struct token *tmp =
				compute_tok(arg, start, i, SPEC_NONE);
			push_back(tokens, tmp);
			free(tmp);
			start = i;
		}
		has_bracket = change_bracket ? !has_bracket : has_bracket;
		spec = is_dollar ? DOLLAR : spec;
		last = compute_tok(arg, start, i, spec);
		start = i + 1;
		is_none = 0;
		is_dollar = 0;
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
		append_vec(tokens, tmp);
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

//============================== PARSER ================================

static enum token_type_spec compute_jt_type(struct vector *subtokens)
{
	for (size_t i = 0; i < subtokens->size; i++) {
		struct token *t = at(subtokens, i);
		switch (t->type_spec) {
		case DSTAR:
			return DSTAR;
		case STAR:
		case SOME:
			return REGEX;
		default:
			break;
		}
	}
	return PATH;
}

static struct wildcard_token *add_token(struct string *buf,
					struct wildcard_token *last,
					struct vector *subtokens)
{
	struct wildcard_token *wt = NULL;
	enum token_type_spec type = compute_jt_type(subtokens);

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
		wt = make_wt(buf, subtokens, type, 1);
	}
	return wt;
}

static void compute_spec_none(struct token *tok, struct string *buffer,
			      struct vector *subtokens)
{
	if (has_bracket) {
		tok->type_spec = SPEC_NONE;
		if (!has_hyphen) {
			append(buffer, tok->data);
			append(buffer, tok->data);
		} else {
			if (size_str(buffer) != 0) {
				pop_back_str(buffer);
				append(buffer, tok->data);
			}
		}
		has_hyphen = 0;
		return;
	}
	append(buffer, tok->data);
	push_back(subtokens, tok);
}

static void compute_star(struct token *tok, struct string *buffer,
			 struct vector *subtokens)
{
	if (has_bracket) {
		if (!has_hyphen) {
			append(buffer, tok->data);
			append(buffer, tok->data);
		} else {
			if (size_str(buffer) != 0) {
				pop_back_str(buffer);
				append(buffer, tok->data);
			}
		}
		return;
	}
	append(buffer, tok->data);
	push_back(subtokens, tok);
}

static void compute_tilde(struct token *tok, struct string *buffer,
			  struct vector *subtokens)
{
	tok->type_spec = SPEC_NONE;
	char *home;
	if (!has_bracket && ((home = getenv("HOME")) != NULL)) {
		clear_str(tok->data);
		append_cstr(tok->data, home);
	}
	compute_spec_none(tok, buffer, subtokens);
}

static void compute_dollar(struct token *tok, struct string *buffer,
			   struct vector *subtokens)
{
	char *data = c_str(tok->data);
	clear_str(tok->data);
	char *var = getenv(data + 1);
	if (var != NULL) {
		append_cstr(tok->data, var);
	}
	free(data);
	if (has_bracket) {
		push_back_str(buffer, '$');
		push_back(subtokens, tok);
	} else {
		tok->type_spec = SPEC_NONE;
		compute_spec_none(tok, buffer, subtokens);
	}
}

static void compute_some(struct string *buffer, struct vector *subtokens)
{
	char *tmp = c_str(buffer);
	struct token *some = make_token(tmp, JOKER, SOME);
	push_back(subtokens, some);
	free_token(some);
	free(tmp);
}

static struct wildcard_token *compute_wildcard_tok(struct token *tok,
						   struct string *buffer,
						   struct wildcard_token *last,
						   struct vector *subtokens)
{
	switch (tok->type_spec) {
	case SLASH:
		return add_token(buffer, last, subtokens);
	case TILDE:
		compute_tilde(tok, buffer, subtokens);
		break;
	case DOLLAR:
		compute_dollar(tok, buffer, subtokens);
		break;
	case RBRACKET:
		if (has_bracket) {
			has_bracket = 0;
			compute_some(buffer, subtokens);
		} else {
			error = ESYNTAX;
		}
	// fall through
	case SPEC_NONE:
		compute_spec_none(tok, buffer, subtokens);
		break;
	case QUESTION_MARK:
	case DSTAR:
	case STAR:
		compute_star(tok, buffer, subtokens);
		break;

	case HYPHEN:
		has_hyphen = 1;
		break;

	case LBRACKET:
		has_bracket = 1;
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
	enum token_type_spec type = compute_jt_type(subtoks);
	struct wildcard_token *tmp = make_wt(buf, subtoks, type, 0);
	push_back(result, tmp);
	free_wt(tmp);
	clear(subtoks);
	clear_str(buf);
}

static struct vector *parse(struct vector *tokens)
{
	has_hyphen = 0;
	has_bracket = 0;
	error = 0;
	if (tokens->size == 0) {
		return NULL;
	}
	struct vector *subtoks = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token,
		(void (*)(void *, void *))copy_token);
	struct wildcard_token *last = NULL;
	struct string *buf = make_string("");
	struct vector *result =
		make_vector(sizeof(struct wildcard_token), destruct_wt,
			    (void (*)(void *, void *))copy_wt);
	size_t i = 0;
	struct token *tok = at(tokens, 0);
	if (tok->type_spec == SLASH) {
		i++;
		struct wildcard_token *wt = make_wt(NULL, NULL, SLASH, 1);
		push_back(result, wt);
		free_wt(wt);
	}
	for (; i < tokens->size && error == 0; i++) {
		tok = at(tokens, i);
		struct wildcard_token *tmp =
			compute_wildcard_tok(tok, buf, last, subtoks);
		if (tmp != NULL) {
			push_back(result, tmp);
			clear(subtoks);
			clear_str(buf);
			free_wt(tmp);
			last = at(result, result->size - 1);
		}
	}
	add_last_token(result, buf, subtoks);
	free_string(buf);
	free_vector(subtoks);
	if (has_hyphen || has_bracket || error) {
		result->free = destruct_wt;
		free_vector(result);
		return NULL;
	}
	return result;
}

static int has_wildcards(struct string *s)
{
	for (size_t i = 0; i < s->cnt->size; i++) {
		switch (compute_token_type(*at_str(s, i))) {
		case SPEC_NONE:
		case HYPHEN:
		case DOLLAR:
		case TILDE:
			break;
		default:
			return 1;
		}
	}
	return 0;
}

//============================ INTERPRETER ==============================

static void add_path(struct wildcard_token *wt, struct vector *result,
		     struct string *path)
{
	struct stat st;
	char *c_path = c_str(path);
	if (size_str(path) > PATH_MAX || stat(c_path, &st) ||
	    (wt->is_dir && S_ISDIR(st.st_mode))) {
		free(c_path);
		return;
	}
	struct token *t = make_token(c_path, ARG, SPEC_NONE);
	push_back(result, t);
	free(t);
	free(c_path);
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
		if (is_dstar && !is_hidden(entry->d_name)) {
			push_back(dstar, s_name);
		}
		if (check_regex(a, entry->d_name)) {
			push_back(entries, s_name);
		}
		free_string(s_name);
	}
}

static void destruct_pair(struct pair *p)
{
	free_string(p->path);
}
static void copy_pair(struct pair *p, struct pair *cp)
{
	cp->path = malloc(sizeof(struct string));
	copy_str(p->path, cp->path);
	cp->index = p->index;
}

static struct pair init_path(struct vector *wtokens)
{
	struct string *path = make_string("");
	if (path == NULL)
		return (struct pair){.path = NULL, .index = 0};
	struct wildcard_token *wt = at(wtokens, 0);
	size_t i = 0;
	if (wt->type == SLASH) {
		i++;
		push_back_str(path, '/');
	}
	wt = at(wtokens, i);
	if (wt->type == PATH) {
		append(path, wt->path);
		push_back_str(path, '/');
		i++;
	}
	return (struct pair){.path = path, .index = i};
}

static struct pair *analyze_tok(int *is_dstar, struct pair *p,
				struct vector *wtokens, struct vector *result)
{
	struct wildcard_token *wt = at(wtokens, p->index);
	if (wt->type == PATH) {
		append(p->path, wt->path);
		p->index++;
		if (p->index >= wtokens->size) {
			char *c_path = c_str(p->path);
			struct stat st;
			if (stat(c_path, &st) == 0)
				add_path(wt, result, p->path); // check exist
			free(c_path);
			return NULL;
		}
		push_back_str(p->path, '/');
		wt = at(wtokens, p->index);
	}

	if (wt->type == DSTAR) {
		p->index++;
		if (p->index >= wtokens->size) {
			struct wildcard_token *tmp = make_star(wt->is_dir);
			push_back(wtokens, tmp);
			free_wt(tmp);
			wt->is_dir = 1;
		}
		*is_dstar = 1;
	}
	return p;
}

static DIR *opendir_path(struct string *path)
{
	DIR *dir = NULL;
	if (size_str(path) == 0) {
		dir = opendir(".");
	} else {
		char *c_path = c_str(path);
		dir = opendir(c_path);
		free(c_path);
		c_path = NULL;
	}
	return dir;
}

static int finish_path(struct pair *p, struct wildcard_token *wt,
		       struct vector *result, size_t incr, size_t len,
		       struct stat st)
{
	if (p->index + incr < len)
		return 0;
	if (!wt->is_dir) {
		char *c_path = c_str(p->path);
		struct token *tok = make_token(c_path, ARG, SPEC_NONE);
		push_back(result, tok);
		free(tok);
		free(c_path);
		return 1;
	} else if (S_ISDIR(st.st_mode)) {
		push_back_str(p->path, '/');
		char *c_path = c_str(p->path);
		struct token *tok = make_token(c_path, ARG, SPEC_NONE);
		push_back(result, tok);
		free(tok);
		free(c_path);
		pop_back_str(p->path);
		return 1;
	}
	return 0;
}

static struct vector *search(struct pair *p, struct vector *queue,
			     struct vector *entries, struct vector *result,
			     int lnk, size_t incr, struct vector *wtokens)
{
	struct wildcard_token *wt = at(wtokens, p->index);
	struct stat st, lst;
	char *c_path = NULL;
	struct string *name = NULL;
	for (size_t j = 0; j < entries->size; j++) {
		name = at(entries, j);
		if (size_str(p->path) + size_str(name) + 2 >= PATH_MAX) {
			continue;
		}
		append(p->path, name);
		c_path = c_str(p->path);
		if (stat(c_path, &st) == -1 || lstat(c_path, &lst) == -1) {
			goto error;
		}
		free(c_path);
		c_path = NULL;
		if (finish_path(p, wt, result, incr, wtokens->size, st)) {
			pop_back_n_str(p->path, size_str(name));
			continue;
		}

		if (S_ISDIR(st.st_mode) && wt->is_dir &&
		    (lnk || (!lnk && !S_ISLNK(lst.st_mode)))) {
			push_back_str(p->path, '/');
			p->index += incr;
			push_back(queue, p);
			p->index -= incr;
			pop_back_str(p->path);
		}
		pop_back_n_str(p->path, size_str(name));
	}
	return queue;
error:
	if (c_path)
		free(c_path);
	return NULL;
}

static struct vector *interpret(struct vector *wtokens)
{
	has_hyphen = 0;
	has_bracket = 0;
	error = 0;
	if (wtokens->size == 0) {
		return NULL;
	}
	DIR *dir = NULL;
	struct vector *entries = NULL;
	struct vector *queue = NULL;
	struct vector *dstar = NULL;
	struct pair first = init_path(wtokens);
	if (first.path == NULL)
		return NULL;
	struct vector *result = make_vector(
		sizeof(struct token), (void (*)(void *))destruct_token, NULL);
	queue = make_vector(sizeof(struct pair),
			    (void (*)(void *))destruct_pair,
			    (void (*)(void *, void *))copy_pair);
	entries = make_vector(sizeof(struct string),
			      (void (*)(void *))destruct_string,
			      (void (*)(void *, void *))copy_str);
	dstar = make_vector(sizeof(struct string),
			    (void (*)(void *))destruct_string,
			    (void (*)(void *, void *))copy_str);
	struct pair *p = malloc(sizeof(struct pair));
	if (!result || !queue || !entries || !dstar || !p)
		goto error_malloc;
	push_back(queue, &first);
	destruct_pair(&first);
	while (queue->size != 0) {
		peek_pop(queue, p);
		int is_dstar = 0;
		if (NULL == analyze_tok(&is_dstar, p, wtokens, result)) {
			destruct_pair(p);
			continue;
		}
		struct wildcard_token *wt = at(wtokens, p->index);
		dir = opendir_path(p->path);
		if ((dir) == NULL) {
			destruct_pair(p);
			continue;
		}
		struct automaton *a = make_automaton(wt->subtokens);
		analyze_dir(dir, a, entries, dstar, is_dstar);
		free_automaton(a);
		closedir(dir);
		if (is_dstar && dstar->size != 0) {
			p->index--;
			search(p, queue, dstar, result, 0, 0, wtokens);
			p->index++;
		}
		search(p, queue, entries, result, 1, 1, wtokens);
		clear(dstar);
		clear(entries);
		destruct_pair(p);
	}
	free_vector(entries);
	free_vector(queue);
	free_vector(dstar);
	free(p);
	return result;

error_malloc:
	if (queue)
		free_vector(queue);
	if (result)
		free_vector(result);
	if (dstar)
		free_vector(dstar);
	if (entries)
		free_vector(entries);
	if (p)
		free(p);
	return NULL;
}

//==================== MAIN FUNCTION =====================

struct vector *expand_wildcards(struct token *tok)
{
	has_hyphen = 0;
	has_bracket = 0;
	error = 0;
	if (!has_wildcards(tok->data))
		return NULL;

	struct vector *l = lex(tok);
	if (l == NULL)
		return NULL;
	struct vector *p = parse(l);
	free_vector(l);
	if (p == NULL)
		return NULL;
	struct vector *result = interpret(p);
	free_vector(p);
	if (result == NULL) {
		return NULL;
	}
	if (result->size == 0) {
		free_vector(result);
		return NULL;
	}
	return result;
}
