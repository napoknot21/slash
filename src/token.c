#include "token.h"

#include "string.h"

#include <stdlib.h>

token *make_token(const char *data, enum token_type type,
		  enum token_type_spec type_spec)
{
	token *new = malloc(sizeof(token));
	if (new == NULL) {
		return NULL;
	}
	new->data = make_string(data);
	if (new->data == NULL) {
		free(new);
		return NULL;
	}
	new->type = type;
	new->type_spec = type_spec;
	return new;
}

void free_token(token *t)
{
	if (t == NULL) {
		return;
	}
	u_free_token(t);
	free(t);
}

void u_free_token(void *t)
{
	if (t == NULL) {
		return;
	}
	free_string(((token *)t)->data);
}
