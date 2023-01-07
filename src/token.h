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
	CONTROL,
	SYNTAX,
	TYPE_NONE, // Default type
	JOKER
};

/**
 * Define a specification for the type. Each specification is for a specific
 * type.
 */
enum token_type_spec {
	/**
	 * CMD specifications
	 */
	INTERNAL,
	EXTERNAL,

	/**
	 * REDIRECT specifications
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
	 * OPERATOR specifications
	 */
	AND,
	OR,
	SEMICOLON,

	/**
	 * ARG specifications
	 */
	SPEC_NONE,

	/**
	 * Joker specifications
	 */
	STAR,
	SOME,
	DSTAR,
	QUESTION_MARK,
	SLASH,
	PATH,
	REGEX,
	DOLLAR,
	TILDE,
	HYPHEN,
	LBRACKET,
	RBRACKET,

	/**
	 * Syntax specification
	 */
	DQUOTE,
	QUOTE,
	ECHAP,

	/**
	 * Control specification
	 */
	IF,
	THEN,
	ELSE,
	DO,
	DONE,
	WHILE,
	FOR,
	IN,
	FI,

	DOLLAR_CMD,
	NOT,

	/**
	 * Other
	 */
	LBRACE,
	RBRACE,
	LCURVE,
	RCURVE,
	DOT,

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
 * @param data The token's value
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
void destruct_token(struct token *t);
void free_token(struct token *t);
void copy_token(const struct token *, struct token *);

#endif
