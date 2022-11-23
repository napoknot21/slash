#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "internals.h"
#include "path.h"
#include "slasherrno.h"

int builtin_pwd (int std, int err, int argc, char **argv)
{
	char *path = getenv("PWD");

	//int kind = LOGICAL_PATH;

	switch (argc) {

		case 1 :
			write (std, path, strlen(path));
			write(std,"\n",1);
			return STATUS_PWD_SUCCESS;

		case 2 :
			if (!strcmp(argv[1], "-L")) {
				write(std, path, strlen(path));
				write(std,"\n",1);
				return STATUS_PWD_SUCCESS;

			} else if (!strcmp(argv[1], "-P")) {
				//kind = PHYSICAL_PATH;

			} else {
				write(err, "pwd: Not valid option!\n", 24);
				return STATUS_PWD_ERROR;
			}
			break;

		default :
			write (err, "pwd: Expected 0 or 1 argument!\n", 32);
			return STATUS_PWD_ERROR;

	}

	/**
	 * Symlinks case
	 */

	char buff[PHYSICAL_PATH_BUFFER];

	//struct string *path_str = make_string(path);

	char *sympath = realpath(path, buff);

	if (sympath == NULL) {
		write (err, "Internal problem with the symbolic path...\n", 44);

		return STATUS_PWD_ERROR;
	}

	write (std, sympath, strlen(sympath));
	write(std,"\n",1);

	return STATUS_PWD_SUCCESS;
}
