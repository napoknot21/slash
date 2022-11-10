#include <string.h>
#include <stdio.h>
#include "test_token.h"
#include "../src/token.h"
#include "testlib.h"

static int test_cmd(char *str, enum token_type type, enum token_type_spec spec)
{
    int bool = 1;
    token *t = token_new(str, type, spec);
    bool &= ASSERT(t->type == type);
    char res[t->data->cnt->size + 1];
    memmove(res, front_str(t->data), t->data->cnt->size);
    res[t->data->cnt->size] = 0;
    bool &= ASSERT(strcmp(str, res) == 0);
    bool &= ASSERT(t->type_spec == spec);
    token_free(t);
    return bool;
}

static int test_new(char *str, enum token_type type, enum token_type_spec spec)
{
    int bool = 1;
    token *t = TOKEN_NEW(str);
    bool &= ASSERT(t->type == type);
    char res[t->data->cnt->size + 1];
    memmove(res, front_str(t->data), t->data->cnt->size);
    res[t->data->cnt->size] = 0;
    bool &= ASSERT(strcmp(str, res) == 0);
    bool &= ASSERT(t->type_spec == spec);
    token_free(t);
    return bool;
}

int test_token()
{
    int bool = 1;
    bool &= ASSERT(test_cmd("cd", CMD, INTERNAL)); // Valid for EXTERNAL
    bool &= ASSERT(test_new("<", REDIRECT, STDIN));
    bool &= ASSERT(test_new(">", REDIRECT, STDOUT));
    bool &= ASSERT(test_new(">|", REDIRECT, STDOUT_TRUNC));
    bool &= ASSERT(test_new(">>", REDIRECT, STDOUT_APPEND));
    bool &= ASSERT(test_new("2>", REDIRECT, ERROUT));
    bool &= ASSERT(test_new("2>|", REDIRECT, ERROUT_TRUNC));
    bool &= ASSERT(test_new("2>>", REDIRECT, ERROUT_APPEND));
    bool &= ASSERT(test_new("|", REDIRECT, PIPE));
    bool &= ASSERT(test_new("I'm an argument", ARG, SPEC_NONE));
    bool &= ASSERT(test_new("I'm", ARG, SPEC_NONE));
    bool &= ASSERT(test_new("I'", ARG, SPEC_NONE));
    bool &= ASSERT(test_new("<2", ARG, SPEC_NONE));
    bool &= ASSERT(test_new("2>g", ARG, SPEC_NONE));
    bool &= ASSERT(test_new(">2>", ARG, SPEC_NONE));
    return bool;
}