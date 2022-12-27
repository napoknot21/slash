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

static int compute_cmd(struct token *tok, struct vector *line, int iscmd);
// static int compute_redirect(token *tok, token *file, int *fdin, int *fdout,
//			    int *fderr);
// static int compute_pipe(token *tok, vector *line, int *fdin, int *pout,
//			int *iscmd);
// static int compute_operator(token *tok, vector *line, int *iscmd);
static int compute_args(struct token *tok, struct vector *line);
static int exec_internal(struct vector *line, int fdout, int fderr);
static char **convertstr(struct vector *line);
static void free_argv(char **argv, size_t size);
static int exec_external(struct vector *line, int fdin, int fdout, int fderr);
static int compute_jokers(struct token *tok, struct vector *line);

static char **convertstr(struct vector *line)
{
	char **argv = calloc(sizeof(*argv), line->size + 1);
	if (argv == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < line->size; i++) {
		struct token *tok = at(line, i);
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

static int compute_jokers(struct token *tok, struct vector *line)
{
	struct vector *jokers = expand_jokers(tok);
	if (jokers == NULL)
		return push_back(line, tok);
	for (size_t i = 0; i < jokers->size; i++) {
		struct token *tok = at(jokers, i);
		push_back(line, tok);
	}
	free_vector(jokers);
	return 0;
}

static int compute_cmd(struct token *tok, struct vector *line, int iscmd)
{
	if (iscmd ||
	    (tok->type_spec != INTERNAL && tok->type_spec != EXTERNAL)) {
		// raise error
		printf("error cmd");
		slasherrno = S_EFAIL;
		return 1;
	}
	int ret = compute_jokers(tok, line);
	struct token *cmd = (struct token *)at(line, 0);
	cmd->type = tok->type;
	char *name = c_str(cmd->data);
	cmd->type_spec = is_internal(name) ? INTERNAL : EXTERNAL;
	free(name);
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

/*static int compute_pipe(token *tok, vector *line, int *fdin, int *pout,
			int *iscmd)
{
	// Changer valeur fdin, pout (open pipe)
	perror("pipe");
	return 0;
}*/

/*static int compute_operator(token *tok, vector *line, int *iscmd)
{
	perror("operator");
	return 0;
}*/

static int compute_args(struct token *tok, struct vector *line)
{
	return compute_jokers(tok, line);
}

static int exec_internal(struct vector *line, int fdout, int fderr)
{
	struct token *name = at(line, 0);
	struct internal c = get_internal(name->data);
	char **argv = convertstr(line);
	if (c.cmd == NULL) {
		return S_ENOCMD;
	}
	if (argv == NULL) {
		return S_EFAIL;
	}
	int ret = c.cmd(fdout, fderr, (int)line->size, argv);
	free_argv(argv, line->size);
	return ret;
}

static int exec_external(struct vector *line, int fdin, int fdout, int fderr)
{
	char **argv = convertstr(line);
	int ret = built_out(fdin, fdout, fderr, line->size, argv);
	free_argv(argv, line->size);
	return ret;
}

int exec(struct vector *line, int fdin, int fdout, int fderr)
{
	if (line->size == 0)
		return slasherrno;
	struct token *cmd = at(line, 0);
	if (cmd == NULL) {
		return -1;
	}
	int ret = 0;
	switch (cmd->type_spec) {
	case INTERNAL:
		ret = exec_internal(line, fdout, fderr);
		break;
	case EXTERNAL:
		ret = exec_external(line, fdin, fdout, fderr);
		break;
	default:
		slasherrno = S_ENOCMD;
		return S_ENOCMD;
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
    switch(line[0]->type)
	case INTERNAL -> map -> cmd
	case EXTERNAL -> fork -> blah blah (charly)
    si ispipe
	fdin = pout;
	pout = -1
	is pipe = false
*/

struct vector *parse(struct vector *tokens)
{
	int iscmd = 0;
	// int pout = -1;
	int ret = 0;

	struct vector *line = make_vector(sizeof(struct token),
					  (void (*)(void *))destruct_token,
					  (void (*)(void *, void *))copy_token);

	for (size_t i = 0; (i < tokens->size) && (ret == 0); i++) {
		struct token *tok = at(tokens, i);
		switch (tok->type) {
		case CMD:
			ret = compute_cmd(tok, line, iscmd);
			iscmd = (ret == 0);
			break;
		case REDIRECT:
			if (tok->type_spec == PIPE) {
				iscmd = 0;
			}
			if (i + 1 >= tokens->size) {
				ret = -1;
				push_back(line, tok);
				slasherrno = S_ELISTENING;
				break;
			}
			push_back(line, tok);
			break;
		case ARG:
			ret = compute_args(tok, line);
			break;
		case OPERATOR:
			push_back(line, tok);
			break;
		default:
			ret = -1;
			slasherrno = S_EUNKNOWN;
			break;
		}
	}
	return line;
}
