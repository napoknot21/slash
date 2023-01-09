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
#include "string.h"
#include "signal.h"

#define RET_LINE "\n"
#define HELP_MESSAGE "pwd: utilisation :pwd [-LP]\n"


int builtin_pwd (int std, int err, int argc, char **argv);
static int logical_path (int std, int err, struct string *path, struct string *retLine);
static int physical_path (int std, int err, struct string *path, struct string *retLine);
static void err_message_argument (int std, int err, struct string *path, struct string *retLine, char *argv);
static void free_main_strings (struct string *path, struct string *retLine);


/**
 * Maiin PWD function
 */
int builtin_pwd (int std, int err, int argc, char **argv)
{

	struct string *path = make_string(getenv("PWD"));
	struct string *retLine = make_string(RET_LINE);

	int kind = LOGICAL_PATH;

	for (int i = 1; i < argc && (interrupt_state == 0) && (sigterm_received == 0); i++) {

		if (!strcmp(argv[i], "-L")) {			
			continue;

		} else if (!strcmp(argv[i], "-P")) {
			kind = PHYSICAL_PATH;

		} else {			
			err_message_argument(std, err, path, retLine, argv[i]);
			return STATUS_PWD_ERROR;
		}
	}

	/**
	 * Default case and logical path (-L option)
	 */
	if (kind == LOGICAL_PATH) {

		return logical_path(std, err, path, retLine);		
	
	}

	/**
	 * Symlinks case (physical path) option -P
	 */
	return physical_path(std, err, path, retLine);
}


/**
 * LOGICAL_PATH argument [-L] for PWD -L 
 */
static int logical_path (int std, int err, struct string *path, struct string *retLine) 
{

	append(path, retLine);

	char *message = c_str(path);

	if (message == NULL) {
		
		write(err, "pwd: Internal issue with the stdout\n", 36);

		free_main_strings (path, retLine);
		
		return STATUS_PWD_ERROR;
	}

	write (std, message, strlen(message));

	free(message);
	free_main_strings (path, retLine);

	return STATUS_PWD_SUCCESS;
}


static int physical_path (int std, int err, struct string *path, struct string *retLine) 
{
	char buff[PHYSICAL_PATH_BUFFER];
	
	char *tmp = c_str(path);
	
	char *sympath = realpath(tmp, buff);

	if (sympath == NULL) {
		
		write (err, "pwd: Internal issue with the symbolic path...\n", 47);
		
		free_main_strings (path, retLine);

		return STATUS_PWD_ERROR;
	}

	struct string *pathsym = make_string(sympath);

	if (pathsym == NULL) {
		write (err, "pwd: Internal issue getting the symlink\n",40);
		
		free_main_strings (path, retLine);

		return STATUS_PWD_ERROR;

	}

	append (pathsym, retLine);

	char *message = c_str(pathsym);

	if (message == NULL) {

		write(err,"pwd: Internal issue with symlink path\n",38);

		free_main_strings (path, retLine);
		free_string(pathsym);

		return STATUS_PWD_ERROR;
	}

	write (std, message, strlen(message));

	free(message);
    free(tmp);
	free_string(pathsym);
	free_main_strings (path, retLine);

	return STATUS_PWD_SUCCESS;
}


/**
 * Functions that write in the stderr the error message for an argument !
 */
static void err_message_argument (int std, int err, struct string *path, struct string *retLine, char *argv) 
{
    struct string *errMessage = make_string("pwd: Invalid option: ");
	struct string *errArg = make_string(argv);
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
		free_main_strings(path, retLine);

		return;
	}

	write (err, message, strlen(message));

	free(message);
	free_string(errMessage);
	free_string(errHelp);
	free_string(errArg);
	free_main_strings (path, retLine);
}


/**
 * Function that free the main string: path and retLine
 */
static void free_main_strings (struct string *path, struct string *retLine)
{
	free_string(retLine);
	free_string(path);
}

