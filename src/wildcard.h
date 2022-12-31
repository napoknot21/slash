#ifndef SLASH_JOKER_H
#define SLASH_JOKER_H

#include "token.h"
#include "vector.h"

/**
 * Expands the jokers according to the given regex in the token data.
 *
 * @param tok The token which store the regex.
*/
struct vector *expand_wildcards(struct token *tok);

#endif
