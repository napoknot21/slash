#include "token.h"

#include "string.h"

#include <stdlib.h>

struct token *make_token(const char *data, enum token_type type,
			 enum token_type_spec type_spec)
{
	struct token *new = malloc(sizeof(*new));
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

void free_token(struct token *t)
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
	free_string(((struct token *)t)->data);
}
