#ifndef SLASH_INTERNALS_H
#define SLASH_INTERNALS_H

#define STATUS_CD_SUCCESS 0
#define STATUS_CD_ERROR 1

#define MAX_EXIT_VAL 255
#define STATUS_EXIT_ERROR 256

#include "string.h"

struct internal {
	char *name;
	int (*cmd)(int fdout, int fderr, int argc, char **argv);
};

extern struct internal internals[];

int is_internal(const char *cmd);
void *get_fonction(struct string *cmd);

int builtin_cd(int, int, int, char **);
int builtin_exit(int, int, int, char **);

#endif
