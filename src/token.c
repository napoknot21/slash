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

void destruct_token(struct token *t)
{
	if (!t)
		return;
	
	free_string(t->data);	
}

void free_token(struct token *t)
{
	destruct_token(t);
	free(t);
}

void copy_token(const struct token * t, struct token * cp)
{	
	cp->data = malloc(sizeof(struct string));
	
	copy_str(t->data, cp->data);	
	cp->type = t->type;
	cp->type_spec = t->type_spec;
}
