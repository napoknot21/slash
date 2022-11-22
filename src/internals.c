#include "internals.h"

#include "string.h"

#include <string.h>

struct internal internals[] = {
	{"cd", builtin_cd}, {"pwd", builtin_pwd}, {"exit", builtin_exit}};

int is_internal(const char *cmd)
{
	size_t inlen = sizeof(internals) / sizeof(internals[0]);
	for (size_t i = 0; i < inlen; i++) {
		if (strcmp(internals[i].name, cmd) == 0) {
			return 1;
		}
	}
	return 0;
}

struct internal get_internal(struct string *cmd)
{
	size_t inlen = sizeof(internals) / sizeof(internals[0]);
	for (size_t i = 0; i < inlen; i++) {
		char *s = c_str(cmd);
		if (strcmp(internals[i].name, s) == 0) {
			free(s);
			return internals[i];
		}
		free(s);
	}
	return INTERNAL_NULL;
}
