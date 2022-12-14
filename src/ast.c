#include "ast.h"

#include "vector.h"
#include "token.h"
#include "token.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
void stack_tokens (struct vector *tokens, struct vector *cmds, struct vector *, struct vector *reds);
*/
struct AST_t * make_ast ()
{
    struct AST_t *root = malloc (sizeof(struct AST_t));
    memset(&root->token, 0, sizeof(struct token));
    //root->size = 0;
    root->childs = make_vector(sizeof(struct AST_t), NULL);
    return root;
}


void free_ast (struct AST_t *root) 
{
    struct AST_t *tmp = root;
    if (tmp->childs != NULL) {
        for (size_t i = 0; i < tmp->childs->size; i++) {
            free_ast(at(tmp->childs,i));
        }
        free_vector(tmp->childs);
    }
    free(root);
}


int add_ast_child (struct AST_t *root, struct token *tok) 
{
    struct AST_t *child = make_ast();
    child->token = *tok;
    int val = push_back(root->childs, child);
    return val;
}


/*
void polonaise (struct vector *token, struct vector *cmds, struct vector *args, struct vector *red) 
{
    size_t fun_tok = -1;
    struct vector fun_childs;

    for (size_t i = 0; i < token->size; i++) {
        if (fun_tok != -1) {
            struct AST_t args_tree;
;        }
    }
    
}
*/

void stack_tokens (struct vector *tokens) 
{
    struct vector *cmds = make_vector(sizeof(struct token), NULL);//destruct_token); //CMDs stack
    struct vector *args = make_vector(sizeof(struct token), NULL);//destruct_token); //ARGS stack
    struct vector *reds = make_vector(sizeof(struct token), NULL);// destruct_token); //REDIRECT stack
    
    for (size_t i = 0; i < tokens->size; i++) {
        
        struct token *tok = at(tokens, i);
        
        switch (tok->type) {
            case CMD :
                push_back(cmds,tok);
                break;

            case REDIRECT :
                if (tok->type_spec == PIPE) {
                    
                }
                push_back(reds, tok);
                break;

            case ARG :
                push_back(args, tok);
                break;

            default :
                //Case for functions (or scripting)
                break;
        }
    }

    free_vector(cmds);
    free_vector(args);
    free_vector(reds);
}

/*
struct AST_t * built_ast_block (struct token *tokens, struct vector *cmds, struct vector *args, struct vector *reds)
{
    
    struct AST_t *root = make_ast();
    
    for (size_t i = 0; i < reds->size; i++) {
        
        root->token = at(reds, i);
    }
    
}
*/

