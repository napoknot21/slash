#include "parser.h"

#include "internals.h"
#include "joker.h"
#include "proc.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int compute_cmd(struct token *tok, struct vector *args, int iscmd);
// static int compute_redirect(token *tok, token *file, int *fdin, int *fdout,
//			    int *fderr);
// static int compute_pipe(token *tok, vector *args, int *fdin, int *pout,
//			int *iscmd);
// static int compute_operator(token *tok, vector *args, int *iscmd);
static int compute_args(struct token *tok, struct vector *args);
static int exec(struct vector *args, int fdin, int fdout, int fderr);
static int exec_internal(struct vector *args, int fdout, int fderr);
static char **convertstr(struct vector *args);
static void free_argv(char **argv, size_t size);
static int exec_external(struct vector *args, int fdin, int fdout, int fderr);
static int compute_jokers(struct token *tok, struct vector *args);

static char **convertstr(struct vector *args)
{
	char **argv = calloc(sizeof(*argv), args->size + 1);
	if (argv == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < args->size; i++) {
		struct token *tok = at(args, i);
		argv[i] = (char *)c_str(tok->data);
		if (argv[i] == NULL) {
			free_argv(argv, i);
			return NULL;
		}
	}
	return argv;
}

static void free_argv(char **argv, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		if (argv[i] != NULL)
			free(argv[i]);
	}
	free(argv);
}

static int compute_jokers(struct token *tok, struct vector *args)
{
	struct vector *jokers = expand_jokers(tok);
	if (jokers == NULL)
		return push_back(args, tok);
	for (size_t i = 0; i < jokers->size; i++) {
		struct token *tok = at(jokers, i);
		push_back(args, tok);
	}
	free_vector(jokers);
	return 0;
}

static int compute_cmd(struct token *tok, struct vector *args, int iscmd)
{
	if (iscmd ||
	    (tok->type_spec != INTERNAL && tok->type_spec != EXTERNAL)) {
		// raise error
		printf("error cmd");
		slasherrno = EFAIL;
		return 1;
	}
	int ret = compute_jokers(tok, args);
	struct token *cmd = (struct token *)at(args, 0);
	cmd->type = tok->type;
	cmd->type_spec = tok->type_spec;
	return ret;
}

/*static int compute_redirect(token *tok, token *file, int *fdin, int *fdout,
			    int *fderr)
{
	//switch(tok->type_spec) { //Generer le fd selon STDIN, STDOUT, STDERR
	//(PAS DE PIPE) default: return 1;
	//}
	perror("redirect");
	return 0;
}*/

/*static int compute_pipe(token *tok, vector *args, int *fdin, int *pout,
			int *iscmd)
{
	// Changer valeur fdin, pout (open pipe)
	perror("pipe");
	return 0;
}*/

/*static int compute_operator(token *tok, vector *args, int *iscmd)
{
	perror("operator");
	return 0;
}*/

static int compute_args(struct token *tok, struct vector *args)
{
	return compute_jokers(tok, args);
}

static int exec_internal(struct vector *args, int fdout, int fderr)
{
	struct token *name = at(args, 0);
	struct internal c = get_internal(name->data);
	char **argv = convertstr(args);
	if (c.cmd == NULL) {
		return ENOCMD;
	}
	if (argv == NULL) {
		return EFAIL;
	}
	int ret = c.cmd(fdout, fderr, (int)args->size, argv);
	free_argv(argv, args->size);
	return ret;
}

static int exec_external(struct vector *args, int fdin, int fdout, int fderr)
{
	char **argv = convertstr(args);
	int ret = built_out(fdin, fdout, fderr, args->size, argv);
	free_argv(argv, args->size);
	return ret;
}

static int exec(struct vector *args, int fdin, int fdout, int fderr)
{
	if (args->size == 0)
		return slasherrno;
	struct token *cmd = at(args, 0);
	if (cmd == NULL) {
		return -1;
	}
	int ret = 0;
	switch (cmd->type_spec) {
	case INTERNAL:
		ret = exec_internal(args, fdout, fderr);
		break;
	case EXTERNAL:
		ret = exec_external(args, fdin, fdout, fderr);
		break;
	default:
		slasherrno = ENOCMD;
		return ENOCMD;
	}
	slasherrno = ret;
	return ret;
}

/*
Seules conditions pour exec sont PIPE et EOL (end of line)
Pour redirect changer in out err en concordance
exec lancera la commande interne/externe avec les bonnes valeurs

pseudo code
fdin = STDIN_FILENO
fdout = STDOUT_FILENO
fderr = STDERR_FILENO
pout = -1
tant que pas PIPE ou \n ou OPERATOR
    si cmd -> computecmd
    si redirect
	si STDIN -> fdin = file
	si STDOUT* -> fdout = file
	si STDERR* -> fderr = file
	si PIPE -> fdout = pipe[1]; exec = 1 (bancal) is pipe
    si arg -> computearg =
	si joker -> push_back all find
	sinon push_back arg

exec() -> function
    switch(args[0]->type)
	case INTERNAL -> map -> cmd
	case EXTERNAL -> fork -> blah blah (charly)
    si ispipe
	fdin = pout;
	pout = -1
	is pipe = false
*/

int parse(struct vector *tokens)
{
	int fdin = STDIN_FILENO;
	int fdout = STDOUT_FILENO;
	int fderr = STDERR_FILENO;
	int iscmd = 0;
	// int pout = -1;
	int ret = 0;

	struct vector *args = make_vector(sizeof(struct token),
					  (void (*)(void *))destruct_token,
					  (void (*)(void *, void *))copy_token);

	for (size_t i = 0; (i < tokens->size) && (ret == 0); i++) {
		struct token *tok = at(tokens, i);
		switch (tok->type) {
		case CMD:
			ret = compute_cmd(tok, args, iscmd);
			iscmd = (ret == 0);
			break;
		case REDIRECT:
			if (tok->type_spec == PIPE) {
				//	ret = compute_pipe(tok, args, &fdin,
				//&pout, 			   &iscmd);
				break;
			}
			if (i + 1 >= args->size) {
				ret = -1;
				break;
			}
			// token *file = at(tokens, i + 1);
			// ret = compute_redirect(tok, file, &fdin, &fdout,
			//		       &fderr);

			break;
		case ARG:
			ret = compute_args(tok, args);
			break;
		case OPERATOR:
			// compute_operator(tok, args, &iscmd);
			break;
		default:
			ret = -1;
			break;
		}
	}
	if (ret == 0)
		ret = exec(args, fdin, fdout, fderr);
	if (args != NULL)
		free_vector(args);
	return (ret && slasherrno == 0) ? 0 : 1;
}
