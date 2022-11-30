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
#define HELP_MESSAGE "pwd: utilisation :pwd [-LP]\n"


int builtin_pwd (int std, int err, int argc, char **argv)
{
	struct string *path = make_string(getenv("PWD"));

	struct string *retLine = make_string(RET_LINE);

	if (path == NULL) {
		write(err, "pwd: Internal issue with PWD variable env\n",43);
		free_string(retLine);
		return STATUS_PWD_ERROR;
	}

	int kind = LOGICAL_PATH;

	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-L")) {
			continue;

		} else if (!strcmp(argv[i], "-P")) {
			kind = PHYSICAL_PATH;

		} else {
			
			struct string *errMessage = make_string("pwd: Invalid option: ");
			struct string *errArg = make_string(argv[i]);
			struct string *errHelp = make_string(HELP_MESSAGE);

			append(errArg, retLine);
			append(errMessage, errArg);
			append(errMessage, errHelp);

			char *message = c_str(errMessage);

			if (message == NULL) {

				write (err, "pwd: Internal issue with the error out\n",39);
				free_string(errMessage);
				free_string(errHelp);
				free_string(errArg);
				free_string(retLine);
				free_string(path);

				return STATUS_PWD_ERROR;
			}

			write (err, message, strlen(message));
			
			free(message);
			free_string(errMessage);
			free_string(errHelp);
			free_string(errArg);
			free_string(retLine);
			free_string(path);
			
			return STATUS_PWD_ERROR;
		}

	}

	/**
	 * Default case and logical path (-L option)
	 */

	if (kind == LOGICAL_PATH) {

		append(path, retLine);

		char *message = c_str(path);

		if (message == NULL) {
			
			write(err, "pwd: Internal issue with the stdout\n", 36);

			free_string(path);
			free_string(retLine);
			
			return STATUS_PWD_ERROR;
		}

		write (std, message, strlen(message));

		free(message);
		free_string(retLine);
		free_string(path);

		return STATUS_PWD_SUCCESS;

	}


	/**
	 * Symlinks case (physical path) option -P
	 */
	
	char buff[PHYSICAL_PATH_BUFFER];
	
	char *tmp = c_str(path);
	
	char *sympath = realpath(tmp, buff);

	free(tmp);

	if (sympath == NULL) {
		
		write (err, "pwd: Internal issue with the symbolic path...\n", 47);
		
		free_string(path);
		free_string(retLine);

		return STATUS_PWD_ERROR;
	}

	struct string *pathsym = make_string(sympath);

	if (pathsym == NULL) {
		write (err, "pwd: Internal issue getting the symlink\n",40);
		
		free_string(path);
		free_string(retLine);

		return STATUS_PWD_ERROR;

	}

	free(sympath);
	
	append (pathsym, retLine);

	char *message = c_str(pathsym);

	if (message == NULL) {

		write(err,"pwd: Internal issue with symlink path\n",38);

		free_string(path);
		free_string(retLine);
		free_string(pathsym);

		return STATUS_PWD_ERROR;
	}

	write (std, message, strlen(message));

	free(message);
	free_string(retLine);
	free_string(path);
	free_string(pathsym);

	return STATUS_PWD_SUCCESS;

}
