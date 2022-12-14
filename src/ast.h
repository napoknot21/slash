#ifndef AST_H
#define AST_H

#include <stdio.h>
#include "token.h"
#include "vector.h"

/**
 * AST: Abstract Syntax Tree
 */
struct AST_t {
  struct token token;
  //size_t size;
  //struct AST_t * child;
  struct vector *childs;
};

struct AST_t * make_ast ();

void free_ast(struct AST_t *ast_root);

int add_ast (struct AST_t *ast_root, struct token token);

void eval_ast (struct AST_t *root_ast);

void polonaise (struct token *token, struct vector *cmds, struct vector *args, struct vector *red);

void stack_token (struct vector *tokens);
#endif
