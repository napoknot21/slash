#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "vector.h"
#include "string.h"

static int RETVAL = 0;

static void parse(vector *tokens){}; // TODO
static vector *lex(char *line);

static int isnextcmd(token *last)
{
    return last == NULL || last->type_spec == PIPE || last->type == OPERATOR;
}

static token *buildtoken(const char *str, token *last)
{
    if (isnextcmd(last))
    {
        enum token_type type = CMD;
        int hashmap_get = 0;
        enum token_type_spec type_spec = (hashmap_get) ? INTERNAL : EXTERNAL;
        return token_new(str, type, type_spec);
    }
    return TOKEN_NEW(str);
}

static vector *lex(char *line)
{
    char *delimeters = " ";
    vector *tokens = make_vector(sizeof(token));
    token *last = NULL;
    if (tokens == NULL)
    {
        goto error;
    }

    char *tmp = strtok(line, delimeters);
    while (tmp != NULL)
    {
        token *tok = buildtoken(tmp, last);

        if (tok == NULL)
        {
            goto error;
        }
        push_back(tokens, tok);
        last = tok;
        tmp = strtok(NULL, delimeters);
    }
    return tokens;

error:
    if (tokens)
    {
        free_vector(tokens);
    }
    if (last)
    {
        token_free(last);
    }

    return NULL;
}

int main()
{
    char *line;
    rl_outstream = stderr;
    while (!RETVAL && (line = readline(NULL)) != NULL)
    {
        add_history(line);
        printf("%s\n", line);
        vector *tokens = lex(line);
        if (tokens == NULL)
        {
            RETVAL = 1;
            break;
        }
        parse(tokens);
        free_vector(tokens);
    }
    rl_clear_history();
    return RETVAL;
}
