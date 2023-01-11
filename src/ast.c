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

#define PIPE_BUF 4096

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

		if(i)
			push_back(argc, &cc);

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

	size_t arg_ptr = 0;

	for(size_t i = 0; i < ops->size; i++) {

		size_t * c = (size_t*) at(argc, i);
		struct ast_t * ast = malloc(ast_s);

		copy_token(at(ops, i), &ast->tok);
		ast->size = *c;

		size_t j = 0;

		if(last) {

			ast->size++;

			ast->childs = malloc(ast->size * ast_s);
			copy_ast(last, ast->childs);
			free_ast(last);

			j++;

		} else
			ast->childs = malloc(ast->size * ast_s);

		for(; j < ast->size && arg_ptr < args->size; j++) {

			copy_token(at(args, arg_ptr++), &ast->childs[j].tok);

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
		/*
		 * Manages pipes
		 */

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

int openfd(char * filename, enum token_type_spec ts, int * in, int * out, int * err)
{
	int in_t = *in, out_t = *out, err_t = *err;
	int ret = 1;

	int * fd = out;
	int flags = 0;

	if(ts >= STDERR && ts <= STDERR_APPEND)
		fd = err;
	if(ts == STDIN) {

		flags = O_RDONLY;
		ret = 0;
		fd = in;

	} else if(ts == STDOUT || ts == STDERR)
		flags = O_WRONLY | O_CREAT | O_EXCL;
	else if(ts == STDOUT_TRUNC || ts == STDERR_TRUNC)
		flags = O_WRONLY | O_TRUNC | O_CREAT;
	else if(ts == STDOUT_APPEND || ts == STDERR_APPEND)
		flags = O_WRONLY | O_APPEND | O_CREAT;
	else
		return -1;

	*fd = open(filename, flags, 0664);
	if(*fd == -1) {

		*in = in_t;
		*out = out_t;
		*err = err_t;
		perror("Impossible d'ouvrir ce fichier");
		slasherrno = S_EFAIL;
		return -1;

	}

	return ret;
}

int process_ast(const struct ast_t * ast, int * in, int * out, int * err)
{
	/*
	 * Starts a new process, creating
	 * a new group
	 */

	int status = 0;

	if(ast->tok.type == ARG) {

		char * cstr = c_str(ast->tok.data);
		write(*out, cstr, ast->tok.data->cnt->size);

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
			process_ast(ast->childs + i, &fds[0], &fds[1], err);
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
				status = sint.cmd(*in, *out, *err, argc, argv);
				break;

			case EXTERNAL:
				status = built_out(*in, *out, *err, argc, argv);
				break;

			default:
				break;

		}

		free_string(cmd);

		for(size_t i = 0; i < argc; i++)
			free(argv[i]);

		free(argv);

	} else if(ast->tok.type == REDIRECT) {

		char * filename = c_str(ast->childs[1].tok.data);
		int s = openfd(filename, ast->tok.type_spec, in, out, err);
		if (s == -1)
			return 1;

		free(filename);

		status = process_ast(ast->childs, in, out, err);
	}

	return status;
}

int ast_is_internal(const struct ast_t * ast)
{
	if(!ast)
		return 0;

	int ret = 0;

	if(ast->tok.type_spec == INTERNAL)
		ret = 1;

	for(size_t i = 0; i < ast->size && !ret; i++)
		ret = ast_is_internal(ast->childs + i);

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

		slasherrno = process_ast(ast->childs, &in, &out, &err);
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

	}

	pid_t * pids = malloc(ast_s * sizeof(pid_t));

	for(size_t k = 0; k < ast_s; k++) {

		pids[k] = fork();
		if(pids[k] == -1) {

			dprintf(err, "AST error: failed at launch!");
			return;

		} else if(!pids[k]) {

			for(size_t i = 0; i < 2 * ast_s; i++) {

				if(fds[i] != STDIN_FILENO && fds[i] != STDOUT_FILENO && i != 2 * k && i != 2 * k + 1)
					close(fds[i]);

			}

			int status = process_ast(ast->childs + k, &fds[2 * k], &fds[2 * k + 1], &err);
			exit(status);

		}
	}

	for(size_t i = 0; i < 2 * ast_s; i++) {

		if(fds[i] != STDIN_FILENO && fds[i] != STDOUT_FILENO)
			close(fds[i]);

	}

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
