#ifndef SLASH_TOKEN_H
#define SLASH_TOKEN_H

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
enum type
{
    CMD,      // Command
    REDIRECT, // Redirection
    ARG,      // Argument
    TYPE_NONE // Default type
};

/**
 * Define a specification for the type. Each specification is for a specific type.
 */
enum type_spec
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
    /**
     * Redirect specification
     */
    ERROUT_TRUNC,
    ERROUT_APPEND,
    PIPE,
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
    char *data;               // token value
    enum type type;           // token type
    enum type_spec type_spec; // type specification
} token;

/**
 * Initialize a new token according to the data, type and type_spec.
 *
 * @param *data The token's value
 * @param type The token's type
 * @param type_spec The token's type specification
 */
token *token_new(const char *data, enum type type, enum type_spec type_spec);

/**
 * Free a token and its value.
 *
 * @param *t The token that will be freed
 */
void token_free(token *t);
#endif
