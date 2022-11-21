#ifndef SLASH_TOKEN_H
#define SLASH_TOKEN_H
#include "string.h"

/**
 * Define the token type
 */
enum token_type {
	CMD,	  // Command
	REDIRECT, // Redirection
	ARG,	  // Argument
	OPERATOR, // Operator
	TYPE_NONE // Default type
};

/**
 * Define a specification for the type. Each specification is for a specific
 * type.
 */
enum token_type_spec {
	/**
	 * CMD specification
	 */
	INTERNAL,
	EXTERNAL,

	/**
	 * REDIRECT specification
	 */
	STDIN,
	STDOUT,
	STDOUT_TRUNC,
	STDOUT_APPEND,
	STDERR,
	STDERR_TRUNC,
	STDERR_APPEND,
	PIPE,

	/**
	 * OPERATOR specification
	 */
	AND,
	OR,
	SEMICOLON,

	/**
	 * ARG specification
	 */
	SPEC_NONE
};

/**
 * Lexer's token
 */

struct token {
	struct string *data;		// token value
	enum token_type type;		// token type
	enum token_type_spec type_spec; // type specification
};

/**
 * Initialize a new token according to the data, type and type_spec.
 *
 * @param *data The token's value
 * @param type The token's type
 * @param type_spec The token's type specification
 */
struct token *make_token(const char *data, enum token_type type,
			 enum token_type_spec type_spec);

/**
 * Free a token and its value.
 *
 * @param *t The token that will be freed
 */
void free_token(struct token *t);
void u_free_token(void *t);

#endif
