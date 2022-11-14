#include <unistd.h>
#include "stdlib.h"
#include "parser.h"
#include "token.h"
#include "vector.h"
#include "string.h"

static int stdin = STDIN_FILENO;
static int stdout = STDOUT_FILENO;
static int stderr = STDERR_FILENO; 
static int computecmd(token *tok, vector *args);
static int computeredirect(token *tok, vector *args);
static int computeoperator(token *tok, vector *args);
static int exec(vector *args);


static int computecmd(token *tok, vector *args) {
    if (tok->type_spec != INTERNAL || tok->type_spec != EXTERNAL) {
        //raise error 
        return 1;
    }
    push_back(args, tok);
    return 0;
}







/*
Seules conditions pour exec sont PIPE et EOL (end of line)
Pour redirect changer in out err en concordance
exec lancera la commande interne/externe avec les bonnes valeurs

pseudo code
stdin = STDIN_FILENO
stdout = STDOUT_FILENO
stderr = STDERR_FILENO
pout = -1
tant que pas PIPE ou \n
    si cmd -> computecmd
    si redirect 
        si STDIN -> stdin = file
        si STDOUT* -> stdout = file
        si STDERR* -> stderr = file
        si PIPE -> stdout = pipe[1]; exec = 1 (bancal) is pipe
    si arg -> computearg = 
        si joker -> push_back all find 
        sinon push_back arg

exec() -> function
    switch(args[0]->type)
        case INTERNAL -> map -> cmd
        case EXTERNAL -> fork -> blah blah (charly)
    si ispipe
        stdin = pout;
        pout = -1
        is pipe = false
*/



void parse(vector *tokens)
{
    /*int iscmd = 0;
    int err = 0;
    vector *args = make_vector(sizeof(token), u_free_token);
    for (size_t i = 0; i < tokens->size && !err; i++)
    {
        token *tok = at(tokens, i);
        switch (tok->type)
        {
        case CMD:
            if (!iscmd)
            {
                err = computecmd(tok, args);
                iscmd = 1;
            }
            else
            { // raise error syntax
            }
            break;
        case REDIRECT:
            if (computeredirect(tok, args) != 0)
            {
                // raise error
            }
            break;
        case ARG: push_back(args, tok); break;
            break;
        case OPERATOR:
            if (computeoperator(tok, args) != 0)
            {
                // raise error
            }
            break;
        default:
            // raise error
            break;
        }
        exec();
        free(tok);
    }*/
}