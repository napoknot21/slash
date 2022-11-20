#include "parser.h"

#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int compute_cmd(token *tok, vector *args, int iscmd);
// static int compute_redirect(token *tok, token *file, int *fdin, int *fdout,
//			    int *fderr);
// static int compute_pipe(token *tok, vector *args, int *fdin, int *pout,
//			int *iscmd);
// static int compute_operator(token *tok, vector *args, int *iscmd);
static int compute_args(token *tok, vector *args);
// static int exec(vector *args, int *fdin, int *fdout, int *fderr, int *pout);
// static int exec_internal(vector *args, int fdout, int fderr);
// static int exec_external(vector *args, int fdin, int fdout, int fderr);

static int compute_cmd(token *tok, vector *args, int iscmd)
{
	if (iscmd || tok->type_spec != INTERNAL || tok->type_spec != EXTERNAL) {
		// raise error
		slasherrno = 1;
		return 1;
	}
	perror("cmd");
	push_back(args, tok);
	return 0;
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

static int compute_args(token *tok, vector *args)
{
	// if has joker
	// then computejoker(tok,arg)
	// else
	perror("pushback");
	push_back(args, tok);
	return 0;
}

/*static int exec_internal(vector *args, int fdout, int fderr)
{
	// token *cmd = at(args,0);
	int (*cmd)(int fdout, int fderr, int argc, char **argv) =
get_fonction(cmd->data)
	// appel fonction (args, fdout);

	perror("internal");
	return 0;
}*/

/*static int exec_external(vector *args, int fdin, int fdout, int fderr)
{
	// token *cmd = at(args, 0);
	// fork
	// dup
	// etc
	perror("external");
	return 0;
}*/

/*static int exec(vector *args, int *fdin, int *fdout, int *fderr, int *pout)
{
	perror("exec");
	token *cmd = at(args, 0);
	if (cmd == NULL) {
		return -1;
	}
	int ret = 0;
	switch (cmd->type_spec) {
	case INTERNAL:
		ret = exec_internal(args, *fdout, *fderr);
	case EXTERNAL:
		ret = exec_external(args, *fdin, *fdout, *fderr);
	default:
		return 1;
	}
	if (ret == 0) {
		clear(args);
	}
	return ret;
}*/

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

int parse(vector *tokens)
{
	// int fdin = STDIN_FILENO;
	// int fdout = STDOUT_FILENO;
	// int fderr = STDERR_FILENO;
	int iscmd = 0;
	// int pout = -1;
	int ret = 0;
	vector *args = make_vector(sizeof(token), NULL);
	for (size_t i = 0; (i < tokens->size) && (ret == 0); i++) {
		token *tok = at(tokens, i);
		switch (tok->type) {
		case CMD:
			ret = compute_cmd(tok, args, iscmd) == 0;
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
	// if (exec(args, &fdin, &fdout, &fderr, &pout) != 0) {
	//	// raise error
	//};
	free_vector(args);
	return (ret && slasherrno == 0) ? 0 : 1;
}
