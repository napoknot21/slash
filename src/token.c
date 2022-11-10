#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "token.h"
#include "string.h"

static enum token_type_spec computeredirect(const char *data, size_t len)
{
    if (len == 1)
    {
        switch (data[0])
        {
        case '<':
            return STDIN;
        case '>':
            return STDOUT;
        case '|':
            return PIPE;
        default:
            return SPEC_NONE;
        }
    }

    if (len == 2)
    {
        if (data[0] == '>' && data[1] == '|')
        {
            return STDOUT_TRUNC;
        }
        if (data[0] == '>' && data[1] == '>')
        {
            return STDOUT_APPEND;
        }
        if (data[0] == '2' && data[1] == '>')
        {
            return ERROUT;
        }
        return SPEC_NONE;
    }

    if (len == 3)
    {
        if (data[0] == '2' && data[1] == '>' && data[2] == '|')
        {
            return ERROUT_TRUNC;
        }
        if (data[0] == '2' && data[1] == '>' && data[2] == '>')
        {
            return ERROUT_APPEND;
        }
    }
    return SPEC_NONE;
}

// TODO: A Remplacer par string
token *token_new(const char *data, enum token_type type, enum token_type_spec type_spec)
{
    token *new = malloc(sizeof(token));
    if (new == NULL)
    {
        perror("token_new");
        return NULL;
    }
    new->data = make_string(data);
    if (new->data == NULL)
    {
        perror("token_new");
        free(new);
        return NULL;
    }
    if (type == CMD)
    {
        new->type = type;
        new->type_spec = type_spec;
    }
    if (type == TYPE_NONE)
    {
        new->type_spec = computeredirect(data, strlen(data));
        new->type = (new->type_spec == SPEC_NONE) ? ARG : REDIRECT;
    }
    return new;
}

void token_free(token *t)
{
    free_string(t->data);
    free(t);
}
