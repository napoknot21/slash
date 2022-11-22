#ifndef SLASH_INTERNALS_H
#define SLASH_INTERNALS_H

#define STATUS_CD_SUCCESS 0
#define STATUS_CD_ERROR 1

#define STATUS_PWD_SUCCESS 0
#define STATUS_PWD_ERROR 1

#define MAX_EXIT_VAL 255
#define STATUS_EXIT_ERROR 256

#include "string.h"

#define INTERNAL_NULL ((struct internal){"", NULL})

struct internal {
	char *name;
	int (*cmd)(int fdout, int fderr, int argc, char **argv);
};

int is_internal(const char *cmd);
struct internal get_internal(struct string *cmd);

int builtin_cd(int, int, int, char **);
int builtin_exit(int, int, int, char **);

#endif
