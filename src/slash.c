#include "lexer.h"
#include "parser.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"
#include "ast.h"
#include "signal.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define C_RED "\001\033[31m\002"
#define C_GREEN "\001\033[32m\002"
#define C_CYAN "\001\033[36m\002"
#define C_CLEAR "\001\033[00m\002"
#define C_BLUE "\001\033[34m\002"
#define PROMPT_SIZE 30
#define EXIT_NAME "exit"
#define VERSION "0.2"

static void print_welcome() {
	int size = 80;
	char buf[size];
	write(STDERR_FILENO, "\n", 1);
	memset(buf, '*', size - 1);
	buf[size -1] = '\n';
	write (STDERR_FILENO, buf, size);
	char line[30];
	sprintf(line ,"Welcome into Slash %s", VERSION);
	int len = strlen(line);
	int n = (size - 3 - len) / 2;
	char tmp[size];
	memset(tmp, ' ', size);
	tmp[0] = '*';
	memcpy(tmp + n, line, len);
	tmp [size - 2] = '*';
	tmp [size - 1] = '\n';
	write(STDERR_FILENO, tmp, size);
	write (STDERR_FILENO, buf, size);
	write(STDERR_FILENO, "\n", 1);
}


static void print_bye() {
	dprintf(STDERR_FILENO ,"exiting...\n\n");
}

static char *compute_prompt()
{
	int color_len = strlen(C_RED) + 1;
	char *p = malloc(sizeof(*p) * ((PROMPT_SIZE) + 4 * (color_len + 1)));
	if (p == NULL) {
		slasherrno = 1;
		return NULL;
	}
	char *color = (slasherrno != 0) ? C_RED : C_GREEN;
	char *pwd = getenv("PWD");
	char err[4];
	if (slasherrno == S_ESIG || sigterm_received || interrupt_state)
		memcpy(err, "SIG", 4);
	else
		sprintf(err, "%d", slasherrno);
	size_t pwdlen = strlen(pwd);
	size_t errlen = strlen(err);
	char *format = "[%s%s%s]%s%s%s%s$ ";
<<<<<<< HEAD
	if (pwdlen + 8 + errlen > PROMPT_SIZE) {
		pwd += pwdlen - PROMPT_SIZE + 7 + errlen;
		sprintf(p, format, color, err, C_CLEAR, C_CYAN, "...", pwd,
=======
	if (pwdlen + 7 + errlen > PROMPT_SIZE) {
		char *tmp = pwd + pwdlen - PROMPT_SIZE + 7 + errlen;
		sprintf(p, format, color, err, C_CLEAR, C_CYAN, "...", tmp,
>>>>>>> fix
			C_CLEAR);
	} else {
		sprintf(p, format, color, err, C_CLEAR, C_CYAN, "", pwd,
			C_CLEAR);
	}
	return p;
}

int proceed_script(const char * path)
{
	int fd = open(path, O_RDONLY);
	if(fd == -1) {

		perror("script\n");
		return -1;

	}

	char buffer[512];
	memset(buffer, 0x0, 512);

	struct string * data = make_string(NULL);

	while(read(fd, buffer, 512) > 0) {

		append_cstr(data, buffer);
		memset(buffer, 0x0, 512);

	}

	char * c_data = c_str(data);

	printf("slash script:\n%s\n", c_data);
	free(c_data);

	return 0;
}

int main(int argc, char ** argv)
{
	set_signal_handler();

	if(argc > 1) {

		proceed_script(argv[1]);
		return slasherrno;

	}

	print_welcome();
	char *line;
	rl_outstream = stderr;
	char *prompt = compute_prompt();
	while ((line = readline(prompt)) != NULL || interrupt_state || sigterm_received) {
		interrupt_state = 0;
		sigterm_received = 0;
		free(prompt);
		prompt = NULL;
		if (!line) continue;
        add_history(line);
		struct vector *tokens = lex(line);
		free(line);
		if (tokens == NULL) {
			continue;
		}
		struct vector *line = parse(tokens);
		free_vector(tokens);
		if (line) {
		//	exec(line, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			struct ast_t * gast = make_gast(line->data, line->size);
			exec_ast(gast, 0, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			free_ast(gast);
			free_vector(line);
		}
		if (is_exit_call)
			break;

		prompt = compute_prompt();
	}
	if (prompt != NULL)
		free(prompt);
	rl_clear_history();
	print_bye();
	return slasherrno;
}
