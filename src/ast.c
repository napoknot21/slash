#include "ast.h"
#include "vector.h"
#include "proc.h"
#include "internals.h"
#include "slasherrno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PIPE_BUF 512

/*
 * Polonaise inversée
 */

int polska(struct token * tokens, size_t size, struct vector * ops, struct vector * args, struct vector * argc)
{
	size_t cc = 0;

	for(size_t i = 0; i < size; i++)
	{
		if(tokens[i].type == ARG) {

			push_back(args, &tokens[i]);
			cc++;
			continue;

		}

		if(i) push_back(argc, &cc);
		push_back(ops, &tokens[i]);

		cc = 0;
	}

	push_back(argc, &cc);	

	return 0;
}

struct ast_t * make_ast(struct token * tokens, size_t size)
{
	struct vector * ops = make_vector(VECTOR_TOKEN_CONSTRUCTOR);
	struct vector * args = make_vector(VECTOR_TOKEN_CONSTRUCTOR);
	struct vector * argc = make_vector(sizeof(size_t), NULL, NULL);
	
	int status = polska(tokens, size, ops, args, argc);	

	if(status) {
		return NULL;
	}

	size_t ast_s = sizeof(struct ast_t);
	struct ast_t * last = NULL;

	for(size_t i = 0; i < ops->size; i++) {

		size_t * c = (size_t*) at(argc, i);
		struct ast_t * ast = malloc(ast_s);	

		copy_token(at(ops, i), &ast->tok);
		ast->size = *c;		

		if(last) {
			
			ast->size++;

			ast->childs = malloc(ast->size * ast_s);
			copy_ast(ast->childs, last);
		
		} else
			ast->childs = malloc(ast->size * ast_s);

		for(size_t j = 0; j < ast->size; j++) {

			copy_token(at(args, j), &ast->childs[j].tok);
			ast->childs[j].size = 0;
			ast->childs[j].childs = NULL;	
		}
		
		last = ast;

	}

	free_vector(ops);
	free_vector(args);
	free_vector(argc);	

	return last;
}

struct ast_t * make_gast(struct token * tokens, size_t size)
{
	size_t ast_s = sizeof(struct ast_t);

	struct ast_t * gast = malloc(ast_s);
	memset(gast, 0x0, ast_s);	

	gast->tok.type = TYPE_NONE;
	gast->tok.type_spec = SPEC_NONE;

	struct vector * asts = make_vector(ast_s, (void (*)(void*)) destruct_ast, (void (*)(void*, void*)) copy_ast);
	size_t from = 0, k, offset = 0;

	for(k = 0; k < size; k++) {			

		int p = tokens[k].type_spec == PIPE;

		if(!p)
			offset++;	

		if(p || k + 1 >= size) {		

			struct ast_t * m = make_ast(tokens + from, offset);		
			push_back(asts, m);	

			free_ast(m);
			from = k + 1;
			offset = 0;
			
		}

	}			

	gast->size = asts->size;
	size_t data_s = gast->size * sizeof(struct ast_t);	

	gast->childs = malloc(data_s);
	
	for(size_t i = 0; i < gast->size; i++)
		copy_ast(at(asts, i), gast->childs + i);		

	free_vector(asts);

	return gast;
}

int process_ast(const struct ast_t * ast, int in, int out, int err) 
{
	/*
	 * Starts a new process, creating
	 * a new group
	 */	

	int status = 0;	

	if(ast->tok.type == ARG) {	

		char * cstr = c_str(ast->tok.data);
		write(out, cstr, ast->tok.data->cnt->size);

		free(cstr);

	} else if(ast->tok.type == CMD) {	

		size_t argc = 1 + ast->size;	
		char ** argv = malloc(argc * sizeof(char*));

		argv[0] = c_str(ast->tok.data);	

		int fds[2];
		status = pipe(fds);

		if(status) {
			perror("AST running\n");
			return 1;
		}

		char * buffer = malloc(PIPE_BUF);	

		for(size_t i = 0; i < argc - 1; i++) {

			memset(buffer, 0x0, PIPE_BUF);
			process_ast(ast->childs + i, fds[0], fds[1], err);	
			ssize_t bs = read(fds[0], buffer, PIPE_BUF);				

			argv[i + 1] = malloc(bs + 1);
			argv[i + 1][bs] = 0;
			memmove(argv[i + 1], buffer, bs);	
		}

		free(buffer);	

		struct string * cmd = make_string(argv[0]);
		struct internal sint;

		switch(ast->tok.type_spec) {

			case INTERNAL:			
				sint = get_internal(cmd);
				status = sint.cmd(out, err, argc, argv);	
				break;

			case EXTERNAL:	
				status = built_out(in, out, err, argc, argv);
				break;

			default:
				break;	

		}

		free_string(cmd);

		for(size_t i = 0; i < argc; i++) 
			free(argv[i]);

		free(argv);
	}

	return status;
}

int ast_is_internal(const struct ast_t * ast)
{
	if(!ast || ast->tok.type == TYPE_NONE)
		return 0;

	char * cmd = c_str(ast->tok.data);	
	int ret = 0;

	if(!strcmp(cmd, "cd") || !strcmp(cmd, "pwd") || !strcmp(cmd, "exit"))
		ret = 1;

	free(cmd);
	return ret;
}

void exec_ast(const struct ast_t * ast, int bin, int in, int out, int err)
{
	if(bin || ast_is_internal(ast->childs)) {

		/*
		 * If slash is executing a slash
		 * program, then we stand on the
		 * main process.
		 */		

		slasherrno = process_ast(ast->childs, in, out, err);
		return;
	}

	/*
	 * Otherwise, we start a new process
	 */	

	size_t ast_s = ast->size;

	// Initializes pipes if there are some

	int * fds = malloc(ast_s * 2 * sizeof(int));

	fds[0] = in;
	fds[ast_s * 2 - 1] = out;	

	for(size_t k = 1; k < ast_s; k++) {	

		int fd[2];
		pipe(fd);

		fds[2 * k - 1] = fd[1];	
		fds[2 * k] = fd[0];

	//	fcntl(fds[2 * k], F_SETFL, O_NONBLOCK);
	}

	pid_t * pids = malloc(ast_s * sizeof(pid_t));
	
	for(size_t k = 0; k < ast_s; k++) {
	
		pids[k] = fork();
		if(pids[k] == -1) {
		
			dprintf(err, "AST error: failed at launch!");	
			return;
		
		} else if(!pids[k]) {
		
			if(k) close(fds[2 * k - 1]);
			process_ast(ast->childs + k, fds[2 * k], fds[2 * k + 1], err);	
			
		}
	}

	for(size_t i = 1; i < ast_s; i++)
		close(fds[i]);	

	free(fds);
	int stat;	

	/*
	 * Waits for the first process of the chain to end
	 */

	for(size_t k = 0; k < ast_s; k++) {
	
		pid_t p = waitpid(pids[k], &stat, 0);
		if(p == -1) {

			perror("Erreur lors de l'exécution des processus\n");
			slasherrno = 1;

			return;

		}

		slasherrno = WEXITSTATUS(stat);

	}
	
	free(pids);
}

void copy_ast(struct ast_t * src, struct ast_t * dst)
{	
	copy_token(&src->tok, &dst->tok);
	dst->size = src->size;

	dst->childs = malloc(dst->size * sizeof(struct ast_t));		

	for(size_t k = 0; k < dst->size; k++)
		copy_ast(src->childs + k, dst->childs + k);	
}

void destruct_ast(struct ast_t * ast)
{	
	if(ast->tok.type != TYPE_NONE) 
		free_string(ast->tok.data);

	if(ast->childs) {

		for(size_t k = 0; k < ast->size; k++)
			destruct_ast(ast->childs + k);

		free(ast->childs);
	}	
}

void free_ast(struct ast_t * ast)
{
	destruct_ast(ast);
	free(ast);
}
