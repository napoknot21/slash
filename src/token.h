enum type 
{
    CMD,REDIRECT,ARG, TYPE_NONE
};

enum type_spec 
{
    INTERNAL,EXTERNAL,STDIN,STDOUT,STDOUT_TRUNC,STDOUT_APPEND,ERROUT, 
    ERROUT_TRUNC, ERROUT_APPEND,PIPE, SPEC_NONE
};

typedef struct token
{
    char *data;
    enum type type;
    enum type_spec type_spec;
} token;

token *token_new(const char *data, enum type type, enum type_spec type_spec);

void token_free(token *t);
