#ifndef SLASH_INTERNALS_H
#define SLASH_INTERNALS_H
#include "string.h"

struct internal {
	char *name;
	int (*cmd)(int fdout, int fderr, int argc, char **argv);
};

extern struct internal internals[];

int is_internal(const char *cmd);
void *get_fonction(string *cmd);

#endif
