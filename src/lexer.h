#ifndef SLASH_LEXER_H
#define SLASH_LEXER_H

#include "vector.h"

/**
 * Cut the line in specific tokens.
 * 
 * @param line The line that will be cut into tokens
*/
vector *lex(char *line);

#endif