#include "internals.h"

#include "string.h"

#include <string.h>

struct internal internals[] = {{"cd", cd}, {"pwd", NULL}, {"exit", NULL}};

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
	char * s = malloc(cmd->cnt->size + 1);

	void * res = NULL;

	for (size_t i = 0; i < inlen; i++) {
		char *s = c_str(cmd);
		if (strcmp(internals[i].name, s) == 0) {
			free(s);
			return internals[i];
		}
	}
	return INTERNAL_NULL;
}
