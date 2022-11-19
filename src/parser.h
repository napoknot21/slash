#ifndef SLASH_PARSER_H
#define SLASH_PARSER_H

#include "vector.h"

/**
 * Parse the tokens and execute their meaning.
 *
 * @param tokens The tokens vector from the lexer
 * @return 1 if an error occured and slasherrno is set, 0 otherwise
 */
int parse(vector *tokens); // TODO

#endif
