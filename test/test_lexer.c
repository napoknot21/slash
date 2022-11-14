#include <string.h>
#include <stdio.h>
#include "test_lexer.h"
#include "../src/token.h"
#include "../src/lexer.h"
#include "../src/vector.h"
#include "testlib.h"

static int isequaltmp(char *a, string *b)
{
    size_t size = size_str(b);
    char res[size];
    memcpy(res, front_str(b), size);
    res[size] = '\0';
    return strcmp(a, res) == 0;
}

static int test_tokendata(token *t, char *str, enum token_type type, enum token_type_spec spec)
{
    int bool = 1;
    bool &= ASSERT(t->type == type);
    bool &= ASSERT(isequaltmp(str, t->data));
    bool &= ASSERT(t->type_spec == spec);
    return bool;
}

static int test_newtokencmd(char *str, enum token_type type, enum token_type_spec spec)
{

    vector *res = lex(str);
    if (res == NULL)
    {
        perror("lex");
        ASSERT(0);
        return 0;
    }
    token *t = at(res, res->size - 1);
    int bool = test_tokendata(t, str, type, spec);
    free_vector(res);
    return bool;
}

static int test_newtoken(char *str, enum token_type type, enum token_type_spec spec)
{
    size_t len = strlen(str);
    char *line = malloc(sizeof(char) * (5 + len));
    if (line == NULL)
    {
        perror("test_newtoken");
        return 1;
    }
    if (!memcpy(line, "cmd ", 4))
    {
        perror("test_newtoken");
        free(line);
        return 1;
    }
    if (!memcpy(line + 4, str, (len + 1) * sizeof(char)))
    {
        perror("test_newtoken");
        free(line);
        return 1;
    }
    vector *res = lex(line);
    if (res == NULL)
    {
        perror("lex");
        ASSERT(0);
        return 0;
    }
    token *t = at(res, res->size - 1);
    int bool = test_tokendata(t, str, type, spec);
    free_vector(res);
    free(line);
    return bool;
}

static int test_lexline(char *line, char **d, enum token_type *t, enum token_type_spec *ts, size_t len)
{
    int bool = 1;
    vector *res = lex(line);
    if (res == NULL)
    {
        ASSERT(0);
        return 0;
    }
    bool &= ASSERT(len == res->size);
    for (size_t i = 0; i < len; i++)
    {
        token *resat = at(res, i);
        bool &= test_tokendata(resat, d[i], t[i], ts[i]);
    }

    free_vector(res);
    return bool;
}

int test_token()
{
    int bool = 1;
    bool &= ASSERT(test_newtokencmd("ls", CMD, EXTERNAL)); // Valid for EXTERNAL //Error
    bool &= ASSERT(test_newtoken("<", REDIRECT, STDIN));
    bool &= ASSERT(test_newtoken(">", REDIRECT, STDOUT));
    bool &= ASSERT(test_newtoken(">|", REDIRECT, STDOUT_TRUNC));
    bool &= ASSERT(test_newtoken(">>", REDIRECT, STDOUT_APPEND));
    bool &= ASSERT(test_newtoken("2>", REDIRECT, STDERR));
    bool &= ASSERT(test_newtoken("2>|", REDIRECT, STDERR_TRUNC));  // ERROR
    bool &= ASSERT(test_newtoken("2>>", REDIRECT, STDERR_APPEND)); // ERROR
    bool &= ASSERT(test_newtoken("|", REDIRECT, PIPE));
    bool &= ASSERT(test_newtoken("I'm", ARG, SPEC_NONE));
    bool &= ASSERT(test_newtoken("I'", ARG, SPEC_NONE));
    bool &= ASSERT(test_newtoken("<2", ARG, SPEC_NONE));
    bool &= ASSERT(test_newtoken("2>g", ARG, SPEC_NONE));
    bool &= ASSERT(test_newtoken(">2>", ARG, SPEC_NONE));
    return bool;
}

int test_lex()
{
    char ***d = malloc(sizeof(char **));
    enum token_type **t = malloc(sizeof(enum token_type *));
    enum token_type_spec **ts = malloc(sizeof(enum token_type_spec *));
    int bool = 1;
    char *line;

    line = "cmd an argument";
    *d = (char *[]){"cmd", "an", "argument"};
    *t = (enum token_type[]){CMD, ARG, ARG};
    *ts = (enum token_type_spec[]){EXTERNAL, SPEC_NONE, SPEC_NONE};
    bool &= ASSERT(test_lexline(line, *d, *t, *ts, 3));

    line = "cmd | cmd";
    *d = (char *[]){"cmd", "|", "cmd"};
    *t = (enum token_type[]){CMD, REDIRECT, CMD};
    *ts = (enum token_type_spec[]){EXTERNAL, PIPE, EXTERNAL};
    bool &= ASSERT(test_lexline(line, *d, *t, *ts, 3));

    line = "cmd arg 2> file";
    *d = (char *[]){"cmd", "arg", "2>", "file"};
    *t = (enum token_type[]){CMD, ARG, REDIRECT, ARG};
    *ts = (enum token_type_spec[]){EXTERNAL, SPEC_NONE, STDERR, SPEC_NONE};
    bool &= ASSERT(test_lexline(line, *d, *t, *ts, 4));

    line = "cmd arg arg | cmd arg | cmd >| file >> log";
    *d = (char *[]){
        "cmd", "arg", "arg", "|", "cmd", "arg", "|",
        "cmd", ">|", "file", ">>", "log"};
    *t = (enum token_type[]){
        CMD, ARG, ARG, REDIRECT, CMD, ARG, REDIRECT,
        CMD, REDIRECT, ARG, REDIRECT, ARG};
    *ts = (enum token_type_spec[]){
        EXTERNAL, SPEC_NONE, SPEC_NONE, PIPE, EXTERNAL, SPEC_NONE, PIPE,
        EXTERNAL, STDOUT_TRUNC, SPEC_NONE, STDOUT_APPEND, SPEC_NONE};
    bool &= ASSERT(test_lexline(line, *d, *t, *ts, 12));

    free(d);
    free(t);
    free(ts);
    return bool;
}