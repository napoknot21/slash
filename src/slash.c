#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "vector.h"
#include "string.h"
#include "lexer.h"
#include "parser.h"

static int RETVAL = 0;

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
