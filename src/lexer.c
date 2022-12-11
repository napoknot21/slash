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
static enum token_type_spec computeredirect(const char *data, size_t len);
/**
 * Compute the token type from the type specification.
 * @param type The token type specification
 */
static enum token_type computetype(enum token_type_spec type);
/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec computedirectone(const char *data);
/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec computedirecttwo(const char *data);
/**
 * Compute the token type specification from the data.
 * @param data The data that will be computed
 */
static enum token_type_spec computedirectthree(const char *data);
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
	       last->type == OPERATOR;
}

static enum token_type_spec computedirectone(const char *data)
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
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec computedirecttwo(const char *data)
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
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec computedirectthree(const char *data)
{
	if (data[0] != '2' || data[1] != '>') {
		return SPEC_NONE;
	}
	switch (data[2]) {
	case '|':
		return STDERR_TRUNC;
	case '>':
		return STDERR_APPEND;
	default:
		return SPEC_NONE;
	}
}

static enum token_type_spec computeredirect(const char *data, size_t len)
{
	switch (len) {
	case 1:
		return computedirectone(data);
	case 2:
		return computedirecttwo(data);
	case 3:
		return computedirectthree(data);
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
	default:
		return ARG;
	}
}

static struct token *buildtoken(const char *str, struct token *last)
{
	enum token_type_spec type_spec = computeredirect(str, strlen(str));
	enum token_type type = computetype(type_spec);
	if (isnextcmd(last)) {
		if (type != ARG || type_spec != SPEC_NONE) {
			printf(" type = %d, spec = %d\n", type, type_spec);
			return NULL; // TODO print err selon le token
		}
		type = CMD;
		type_spec = (is_internal(str)) ? INTERNAL : EXTERNAL;
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

	struct vector *tokens = make_vector(
			sizeof(struct token), 
			(void (*)(void*)) destruct_token, 
			(void (*)(void*, void*)) copy_token);

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
