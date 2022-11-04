enum type 
{
    CMD,REDIRECT,ARGS, TYPE_NONE
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

token *token_new(char *data, enum type type);

void token_free(token *t);
