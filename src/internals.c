#include "internals.h"

#include "string.h"

#include <string.h>

struct internal internals[] = {{"cd", NULL}, {"pwd", NULL}, {"exit", NULL}};

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

void *get_fonction(struct string *cmd)
{
	size_t inlen = sizeof(internals) / sizeof(internals[0]);
	for (size_t i = 0; i < inlen; i++) {
		char s[cmd->cnt->size+1];
		memcpy(s,front_str(cmd), cmd->cnt->size);
		s[cmd->cnt->size] = '\0';
		if (strcmp(internals[i].name, s) == 0) {
			free(s);
			return internals[i].cmd;
		}
		free(s);
	}
	return NULL;
}