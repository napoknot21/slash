#include "internals.h"
#include "slasherrno.h"
#include "path.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int cd(int in, int out, int argc, char ** argv)
{
	char * path = getenv("HOME"),
	     * pwd  = getenv("PWD");

	int kind = LOGICAL_PATH;

	/*
	 * This command admits the following options :
	 * 	- logical path (-L) [default]
	 * 	- physical path (-P)
	 */		

	for(int k = 1; k < argc; k++) {

		if(!strcmp(argv[k], "-L")) {
		
			continue;
		
		} else if(!strcmp(argv[k], "-P")) {
			
			kind = PHYSICAL_PATH;
		
		}
		else {
		
			path = argv[k];
				
		}
	}

	struct string * path_str = make_string(path),
		      * pwd_str  = make_string(pwd);

	struct string * dir = normalize_path(path_str, pwd_str);

	/*
	 * We check whether that directory exists,
	 * or is a symlink
	 */

	const char * dir_cstr = c_str(dir);
	int fd = open(dir_cstr, 0);

	free_string(path_str);
	free_string(pwd_str);

	free_string(dir);

	int status = STATUS_CD_SUCCESS;

	/*
	 * That path does not exist
	 */

	if(fd == -1) {
		status = STATUS_CD_ERROR;
		perror("cd: ");
	} else {
		setenv("PWD", dir_cstr, 1);
	}

	close(fd);

	return status;
}
