#include "parser.h"

#include "internals.h"
#include "proc.h"
#include "slasherrno.h"
#include "string.h"
#include "token.h"
#include "vector.h"
#include "wildcard.h"
#include "signal.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int compute_cmd(struct token *tok, struct vector *line, int iscmd);
static int compute_args(struct token *tok, struct vector *line);
static int exec_internal(struct vector *line, int fdout, int fderr);
static char **convertstr(struct vector *line);
static void free_argv(char **argv, size_t size);
static int exec_external(struct vector *line, int fdin, int fdout, int fderr);
static int compute_jokers(struct token *tok, struct vector *line);

static int need_do = 0;
static int need_in = 0;
static int need_done = 0;
static int need_then = 0;
static int need_else = 0;
static int need_fi = 0;
static int need_curve = 0;
static int need_quote = 0;
static int need_dquote = 0;

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

static void expand_var(struct token *tok)
{
	char *s = c_str(tok->data);
	int size = size_str(tok->data);
	clear_str(tok->data);
	char buf[NAME_MAX];
	int start = 0;
	int is_var = 0;
	for (int i = 0; i < size; i++) {
		if (!isalpha(s[i])) {
			memcpy(buf, s + start, i - start);
			buf[i - start] = '\0';
			if (is_var)
				append_cstr(tok->data, getenv(buf));
			else
				append_cstr(tok->data, buf);
			is_var = s[i] == '$';
			start = i + is_var;
		}
	}
	memcpy(buf, s + start, size - start);
	buf[size - start] = '\0';
	if (is_var)
		append_cstr(tok->data, getenv(buf));
	else
		append_cstr(tok->data, buf);
	free(s);
}

static int compute_jokers(struct token *tok, struct vector *line)
{
	struct vector *wildcards = expand_wildcards(tok);
	if (wildcards == NULL) {
		expand_var(tok);
		return push_back(line, tok);
	}
	for (size_t i = 0; i < wildcards->size && (interrupt_state == 0 && sigterm_received == 0); i++) {
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
		slasherrno = S_EFAIL;
		return 1;
	}
	int ret = compute_jokers(tok, line);
	struct token *cmd = (struct token *)at(line, line->size - 1);
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

static int check_control(struct token *tok, size_t i, struct vector *tokens)
{
	int end = i + 1 >= tokens->size ||
		  ((struct token *)at(tokens, i + 1))->type == OPERATOR;
	switch (tok->type_spec) {
	case IF:
		need_then++;
		return !end;
	case WHILE:
		need_do++;
		return !end;
	case DO:
		need_do--;
		need_done++;
		return !end && need_do >= 0;
	case FOR:
		need_in++;
		return !end && ((struct token *)at(tokens, i + 1))->type == ARG;
	case THEN:
		need_else++;
		need_fi++;
		need_then--;
		return !end && need_then >= 0;
	case ELSE:
		need_else--;
		return !end && need_else >= 0;
	case IN:
		need_do++;
		need_in--;
		return !end && need_in >= 0;
	case NOT:
		return !end;
	case DONE:
		need_done--;
		return end && need_done >= 0;
	case DOLLAR_CMD:
		need_curve++;
		return !end;
	case FI:
		need_else = 0;
		need_fi--;
		return end && need_fi >= 0;
	default:
		return 1;
	}
}

static int check_syntax(struct token *tok)
{
	switch (tok->type_spec) {
	case QUOTE:
		if (!need_dquote)
			need_quote = !need_quote;
		break;
	case DQUOTE:
		if (!need_quote)
			need_dquote = !need_dquote;
		break;

	default:
		break;
	}
	return need_dquote || need_quote;
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

static int something_is_missing()
{
	return need_do || need_in || need_done || need_then || need_else ||
	       need_quote || need_curve || need_dquote || need_fi;
}

struct vector *parse(struct vector *tokens)
{
	if(!tokens->size)
		return NULL;

	need_fi = 0;
	need_do = 0;
	need_in = 0;
	need_done = 0;
	need_then = 0;
	need_else = 0;
	need_curve = 0;
	need_quote = 0;
	need_dquote = 0;
	int iscmd = 0;
	int ret = 0;

	struct vector *line = make_vector(sizeof(struct token),
					  (void (*)(void *))destruct_token,
					  (void (*)(void *, void *))copy_token);

	for (size_t i = 0; (i < tokens->size) && (ret == 0) && (interrupt_state == 0 && sigterm_received == 0); i++) {
		struct token *tok = at(tokens, i);
		if ((need_dquote && tok->type_spec != DQUOTE) ||
		    (need_quote && tok->type_spec != QUOTE)) {
			struct token *last = at(line, line->size - 1);
			append(last->data, tok->data);
			push_back_str(last->data, ' ');
			continue;
		}
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
				slasherrno = S_ESYNTAX;
				break;
			}
			push_back(line, tok);
			break;
		case ARG:
			ret = compute_args(tok, line);
			iscmd = 0;
			break;
		case SYNTAX:
			ret = check_syntax(tok);
			if (ret) {
				clear_str(tok->data);
				tok->type = ARG;
				tok->type_spec = SPEC_NONE;
				push_back(line, tok);
			} else {
				struct token *last = at(line, line->size - 1);
				pop_back_str(last->data);
			}
			ret = 0;
			iscmd = 0;

			break;
		case CONTROL:
			ret = !check_control(tok, i, tokens);
			slasherrno = ret != 0 ? S_ESYNTAX : slasherrno;
			iscmd = 0;
		// fall through
		case OPERATOR:
			push_back(line, tok);
			iscmd = 0;
			break;
		default:
			slasherrno = S_EUNKNOWN;
			free_vector(line);
			return NULL;
			break;
		}
	}
	if (ret || something_is_missing()) {
		slasherrno = S_ESYNTAX;
		werror();
		free_vector(line);
		return NULL;
	}
	return line;
}
