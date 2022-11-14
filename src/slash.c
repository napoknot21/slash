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

#define C_RED "\x1B[31m"
#define C_GREEN "\x1B[32m"
#define C_CLEAR "\x1B[0m"

static int RETVAL = 0;

int main()
{
    char *line;
    rl_outstream = stderr;
    char *prompt = "[\x1B[31m1\x1B[0m]$";
    while (!RETVAL && (line = readline(prompt)) != NULL)
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
