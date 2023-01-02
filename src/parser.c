#include "parser.h"

#include "internals.h"
#include "proc.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"
#include "wildcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int compute_cmd(struct token *tok, struct vector *line, int iscmd);
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
	struct vector *wildcards = expand_wildcards(tok);
	if (wildcards == NULL)
		return push_back(line, tok);
	for (size_t i = 0; i < wildcards->size; i++) {
		struct token *tok = at(wildcards, i);
		push_back(line, tok);
	}
	free_vector(wildcards);
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
