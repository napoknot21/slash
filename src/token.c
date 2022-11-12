#include <stdlib.h>
#include "token.h"
#include "string.h"

// TODO: A Remplacer par string
token *token_new(const char *data, enum token_type type, enum token_type_spec type_spec)
{
    token *new = malloc(sizeof(token));
    if (new == NULL)
    {
        return NULL;
    }
    new->data = make_string(data);
    if (new->data == NULL)
    {
        free(new);
        return NULL;
    }
    new->type = type;
    new->type_spec = type_spec;
    return new;
}

void token_free(void *t)
{
    if (t == NULL)
    {
        return;
    }
    free_string(((token *)t)->data);
    free(t);
}
