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

#define RET_LINE "\n"

int builtin_pwd (int std, int err, int argc, char **argv)
{
	struct string *path = make_string(getenv("PWD"));

	struct string *retLine = make_string(RET_LINE);

	if (path == NULL) {
		write(err, "pwd: Internal issue with PWD variable env\n",43);
		return STATUS_PWD_ERROR;
	}

	switch (argc) {
		
		case 1 :
			append(path, retLine);
			char *message = c_str(path);
			write (std, message, strlen(message));
			free(message);
			free_string(retLine);
			free_string(path);
			return STATUS_PWD_SUCCESS;

		case 2 :
			if (!strcmp(argv[1], "-L")) {
				append(path, retLine);
				char *message = c_str(path);
				write(std, message, strlen(message));
				free(message);
				free_string(retLine);
				free_string(path);
				return STATUS_PWD_SUCCESS;
			
			} else if (!strcmp(argv[1], "-P")) {
				//kind = PHYSICAL_PATH;

			} else {
				write(err, "pwd: Not valid option!\n", 23);
				free_string(retLine);
				free_string(path);
				return STATUS_PWD_ERROR;
			}
			break;

		default :
			write (err, "pwd: Expected 0 or 1 argument!\n", 31);
			free_string(retLine);
			free_string(path);
			return STATUS_PWD_ERROR;
			
	}		

	/**
	 * Symlinks case
	 */
	
	char buff[PHYSICAL_PATH_BUFFER];
	
	char *sympath = realpath(c_str(path), buff);

	if (sympath == NULL) {
		write (err, "pwd: Internal issue with the symbolic path...\n", 47);
		free_string(path);
		return STATUS_PWD_ERROR;
	}

	struct string *pathsym = make_string(sympath);
	
	append (pathsym, retLine);

	char *message = c_str(pathsym);

	write (std, message, strlen(message));

	free(message);
	free_string(retLine);
	free_string(path);
	free_string(pathsym);

	return STATUS_PWD_SUCCESS;
}
