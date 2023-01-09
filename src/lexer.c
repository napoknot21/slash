#include "internals.h"
#include "map.h"
#include "token.h"
#include "vector.h"

#include <stdio.h>
#include <string.h>

/**
 * Check if the next token will be a command.
 * @param last Represent the last computed token
 */
static int isnextcmd(struct token *last);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 * @param len The data length
 */
static enum token_type_spec compute_spec(const char *data, size_t len);

/**
 * Compute the token type from the type specification.
 * @param type The token type specification
 */
static enum token_type computetype(enum token_type_spec type);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec compute_spec_1(const char *data);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec compute_spec_2(const char *data);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec compute_spec_3(const char *data);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec compute_spec_4(const char *data);

/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec compute_spec_5(const char *data);

/**
 * Build the right token according to the given string and
 * the last computed token.
 * @param str The token content
 * @param last The last computed token
 */
static struct token *buildtoken(const char *str, struct token *last);

static char *cpy(char *src);

static int isnextcmd(struct token *last)
{
	return last == NULL || last->type_spec == PIPE ||
	       last->type == OPERATOR || last->type_spec == DO ||
	       last->type_spec == THEN || last->type_spec == ELSE ||
	       last->type_spec == DOLLAR_CMD;
}

static enum token_type_spec compute_spec_1(const char *data)
{
	switch (data[0]) {
	case '<':
		return STDIN;
	case '>':
		return STDOUT;
	case '|':
		return PIPE;
	case ';':
		return SEMICOLON;
	case '!':
		return NOT;
	case '"':
		return DQUOTE;
	case '\'':
		return QUOTE;
	case '[' :
		return LBRACKET;
	case ']':
		return RBRACKET;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec compute_spec_2(const char *data)
{
	switch (data[0]) {
	case '>': {
		switch (data[1]) {
		case '|':
			return STDOUT_TRUNC;
		case '>':
			return STDOUT_APPEND;
		default:
			return SPEC_NONE;
		}
	}
	case '2':
		return (data[1] == '>') ? STDERR : SPEC_NONE;
	case '&':
		return (data[1] == '&') ? AND : SPEC_NONE;
	case '|':
		return (data[1] == '|') ? OR : SPEC_NONE;
	case 'f':
		return (data[1] == 'i') ? FI : SPEC_NONE;
	case 'd':
		return (data[1] == 'o') ? DO : SPEC_NONE;
	case 'i':
		switch (data[1]) {
		case 'f':
			return IF;
		case 'n':
			return IN;
		default:
			return SPEC_NONE;
		}
	case '$':
		return (data[1] == '(') ? DOLLAR_CMD : SPEC_NONE;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec check_redirect(const char *data)
{
	if (data[0] != '>') {
		return SPEC_NONE;
	}
	switch (data[1]) {
	case '|':
		return STDERR_TRUNC;
	case '>':
		return STDERR_APPEND;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec compute_spec_3(const char *data)
{
	switch (data[0]) {
	case '2':
		return check_redirect(data + 1);
	case 'f':
		return strcmp(data + 1, "or") == 0 ? FOR : SPEC_NONE;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec compute_spec_4(const char *data)
{
	switch (data[0]) {
	case 't':
		return strcmp(data + 1, "hen") == 0 ? THEN : SPEC_NONE;
	case 'e':
		return strcmp(data + 1, "lse") == 0 ? ELSE : SPEC_NONE;
	case 'd':
		return strcmp(data + 1, "one") == 0 ? DONE : SPEC_NONE;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec compute_spec_5(const char *data)
{
	return strcmp(data, "while") == 0 ? WHILE : SPEC_NONE;
}

static enum token_type_spec compute_spec(const char *data, size_t len)
{
	switch (len) {
	case 1:
		return compute_spec_1(data);
	case 2:
		return compute_spec_2(data);
	case 3:
		return compute_spec_3(data);
	case 4:
		return compute_spec_4(data);
	case 5:
		return compute_spec_5(data);
	default:
		return SPEC_NONE;
	}
}

static enum token_type computetype(enum token_type_spec type)
{
	switch (type) {
	case INTERNAL:
	case EXTERNAL:
		return CMD;

	case AND:
	case OR:
	case SEMICOLON:
		return OPERATOR;

	case STDIN:
	case PIPE:
	case STDOUT:
	case STDOUT_APPEND:
	case STDOUT_TRUNC:
	case STDERR:
	case STDERR_APPEND:
	case STDERR_TRUNC:
		return REDIRECT;

	case NOT:
	case LBRACKET:
	case RBRACKET:
	case IF:
	case THEN:
	case ELSE:
	case WHILE:
	case DO:
	case DONE:
	case FOR:
	case FI:
	case IN:
	case DOLLAR_CMD:
		return CONTROL;

	case DQUOTE:
	case QUOTE:
		return SYNTAX;

	default:
		return ARG;
	}
}

static int need_to_be_first(enum token_type_spec t)
{
	switch (t) {
	case IF:
	case THEN:
	case WHILE:
	case FOR:
	case DO:
	case FI:
	case DONE:
	case ELSE:
		return 1;
	default:
		return 0;
	}
}

static struct token *buildtoken(const char *str, struct token *last)
{
	enum token_type_spec type_spec = compute_spec(str, strlen(str));
	enum token_type type = computetype(type_spec);
	if ((last && last->type_spec != SEMICOLON) && (type == CONTROL) &&
	    need_to_be_first(type_spec)) {
		type = ARG;
		type_spec = SPEC_NONE;
	}
	if (isnextcmd(last) && type == ARG && type_spec == SPEC_NONE) {
		type_spec = INTERNAL;
		type = CMD;
	}
	struct token *t = make_token(str, type, type_spec);
	return t;
}

static char *cpy(char *src)
{
	size_t len = strlen(src);
	char *line = malloc(sizeof(char) * (len + 1));
	if (line == NULL) {
		return NULL;
	}
	if (memcpy(line, src, sizeof(char) * (len)) == NULL) {
		free(line);
		return NULL;
	}
	line[len] = '\0';
	return line;
}

struct vector *lex(char *line)
{
	line = cpy(line);
	char *delimeters = " ";

	struct vector *tokens = make_vector(sizeof(struct token),
					    (void (*)(void *))destruct_token,
					    (void (*)(void *, void *))NULL);

	struct token *last = NULL;

	if (tokens == NULL) {
		goto error;
	}

	char *tmp = strtok(line, delimeters);
	while (tmp != NULL) {
		struct token *tok = buildtoken(tmp, last);

		if (tok == NULL) {
			goto error;
		}
		push_back(tokens, tok);
		if (last != NULL)
			free(last);
		last = tok;
		tmp = strtok(NULL, delimeters);
	}
	free(last);
	free(line);
	return tokens;

error:
	if (tokens) {
		free_vector(tokens);
	}
	if (last) {
		free_token(last);
	}
	if (line) {
		free(line);
	}
	return NULL;
}
