#include <string.h>
#include "test_token.h"
#include "../src/token.h"
#include "testlib.h"

static int test_cmd(char *str, enum type type, enum type_spec spec)
{
    int bool = 1;
    token *t = token_new(str, type, spec);
    bool &= ASSERT(t->type == type);
    bool &= ASSERT(strcmp(str, t->data) == 0);
    bool &= ASSERT(t->type_spec == spec);
    token_free(t);
    return bool;
}

static int test_new(char *str, enum type type, enum type_spec spec)
{
    int bool = 1;
    token *t = TOKEN_NEW(str);
    bool &= ASSERT(t->type == type);
    bool &= ASSERT(strcmp(str, t->data) == 0);
    bool &= ASSERT(t->type_spec == spec);
    token_free(t);
    return bool;
}

int test_token()
{
    int bool = 1;
    bool &= test_cmd("cd", CMD, INTERNAL); // Valid for EXTERNAL
    bool &= test_new("<", REDIRECT, STDIN);
    bool &= test_new(">", REDIRECT, STDOUT);
    bool &= test_new(">|", REDIRECT, STDOUT_TRUNC);
    bool &= test_new(">>", REDIRECT, STDOUT_APPEND);
    bool &= test_new("2>", REDIRECT, ERROUT);
    bool &= test_new("2>|", REDIRECT, ERROUT_TRUNC);
    bool &= test_new("2>>", REDIRECT, ERROUT_APPEND);
    bool &= test_new("|", REDIRECT, PIPE);
    bool &= test_new("I'm an argument", ARG, SPEC_NONE);
    bool &= test_new("I'm", ARG, SPEC_NONE);
    bool &= test_new("I'", ARG, SPEC_NONE);
    bool &= test_new("<2", ARG, SPEC_NONE);
    bool &= test_new("2>g", ARG, SPEC_NONE);
    bool &= test_new(">2>", ARG, SPEC_NONE);
    return bool;
}