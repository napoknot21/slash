#include "lexer.h"
#include "parser.h"
#include "string.h"
#include "token.h"
#include "vector.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_RED "\x1B[31m"
#define C_GREEN "\x1B[32m"
#define C_CLEAR "\x1B[0m"

int main()
{
	int retval = 0;
	char *line;
	rl_outstream = stderr;
	char *prompt = "[\x1B[31m1\x1B[0m]$";
	while (!retval && (line = readline(prompt)) != NULL) {
		add_history(line);
		printf("%s\n", line);
		vector *tokens = lex(line);
		if (tokens == NULL) {
			retval = 1;
			break;
		}
		retval = parse(tokens);
		free_vector(tokens);
	}
	rl_clear_history();
	return retval;
}
