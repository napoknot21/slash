#include "lexer.h"
#include "parser.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_RED "\001\033[31m\002"
#define C_GREEN "\001\033[32m\002"
#define C_CYAN "\001\033[36m\002"
#define C_CLEAR "\001\033[00m\002"
#define C_BLUE "\001\033[34m\002"
#define PROMPT_SIZE 30

static char *compute_prompt();

static char *compute_prompt()
{
	int color_len = strlen(C_RED);
	char *p = malloc(sizeof(*p) * ((PROMPT_SIZE) + 4 * color_len + 1));
	if (p == NULL) {
		slasherrno = 1;
		return NULL;
	}
	char *color = (slasherrno != 0) ? C_RED : C_GREEN;
	char *pwd = getenv("PWD");
	char err[4];
	sprintf(err, "%d", slasherrno);
	size_t pwdlen = strlen(pwd);
	size_t errlen = strlen(err);
	char *format = "[%s%d%s]%s%s%s%s$ ";
	if (pwdlen + 8 + errlen > PROMPT_SIZE) {
		char *tmp = pwd + pwdlen - PROMPT_SIZE + 8;
		sprintf(p, format, color, slasherrno, C_CLEAR, C_CYAN, "...",
			tmp, C_CLEAR);
	} else {
		sprintf(p, format, color, slasherrno, C_CLEAR, C_CYAN, pwd,
			C_CLEAR);
	}
	return p;
}

int main()
{
	int retval = 0;
	char *line;
	rl_outstream = stderr;
	char *prompt = compute_prompt();
	while ((line = readline(prompt)) != NULL) {
		free(prompt);
		add_history(line);
		vector *tokens = lex(line);
		free(line);
		if (tokens == NULL) {
			retval = 1;
			break;
		}
		retval = parse(tokens);
		free_vector(tokens);
		prompt = compute_prompt();
	}
	rl_clear_history();
	return retval;
}
