#ifndef AST_H
#define AST_H

#include "vector.h"
#include "token.h"

/*
 * Abstract Syntaxic Tree
 *
 * Global AST is sepcified by
 * initializing 0 in tok's memory.
 *
 * GAST allows to manage several processus
 * exchanging between them through pipes
 */

struct ast_t 
{
	struct token tok;

	size_t size;
	struct ast_t * childs;	
};

/*
 * Constructors
 */

struct ast_t * make_ast(const struct token *, size_t);
struct ast_t * make_gast(const struct token *, size_t);

/*
 * Modifiers
 */

void copy_ast(const struct ast_t*, struct ast_t*);

/*
 * Observers
 */

int process_ast(const struct ast_t *, int, int, int);
void exec_ast(const struct ast_t *, int, int, int, int);

/*
 * Destructors
 */

void destruct_ast(struct ast_t *);
void free_ast(struct ast_t *);

#endif
