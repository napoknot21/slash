#ifndef SLASH_TOKEN_H
#define SLASH_TOKEN_H
#include "string.h"

/**
 * The only purpose of this macro is to simplify the syntax
 * for ARG and REDIRECT tokens.
 *
 * @param data a char * where the token's data is
 */
#define TOKEN_NEW(data) (token_new((data), TYPE_NONE, SPEC_NONE))

/**
 * Define the token type
 */
enum token_type
{
    CMD,      // Command
    REDIRECT, // Redirection
    ARG,      // Argument
    OPERATOR, // Operator
    TYPE_NONE // Default type
};

/**
 * Define a specification for the type. Each specification is for a specific type.
 */
enum token_type_spec
{
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
    ERROUT,
    ERROUT_TRUNC,
    ERROUT_APPEND,
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
typedef struct token
{
    string *data;                   // token value
    enum token_type type;           // token type
    enum token_type_spec type_spec; // type specification
} token;

/**
 * Initialize a new token according to the data, type and type_spec.
 *
 * @param *data The token's value
 * @param type The token's type
 * @param type_spec The token's type specification
 */
token *token_new(const char *data, enum token_type type, enum token_type_spec type_spec);

/**
 * Free a token and its value.
 *
 * @param *t The token that will be freed
 */
void token_free(token *t);
#endif
