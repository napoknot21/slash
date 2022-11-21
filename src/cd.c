#include "internals.h"
#include "slasherrno.h"
#include "path.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int builtin_cd(int in, int out, int argc, char ** argv)
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
			break;		
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

	struct stat dirstat;
	int stat_s = lstat(dir_cstr, &dirstat);

	/*
	 * Failed to retrieve informations on the node
	 */

	if(stat_s == -1) {

		write(out, "cd: That directory does not exist!\n", 36);
		return STATUS_CD_ERROR;

	}

	mode_t dirmode = dirstat.st_mode;

	/*
	 * Not a directory
	 */

	int symlink = S_ISLNK(dirmode);

	if(!S_ISDIR(dirmode) && !symlink) {
	
		write(out, "cd: Not a directory!\n", 22);
		return STATUS_CD_ERROR;

	}	

	const char * phys_dir_cstr = dir_cstr;
	if(symlink && kind == PHYSICAL_PATH) {	

		/*
		 * We seek the physical path
		 */

		char buff[PHYSICAL_PATH_BUFFER];
		memset(buff, 0x0, PHYSICAL_PATH_BUFFER);

		ssize_t rds = readlink(dir_cstr, buff, PHYSICAL_PATH_BUFFER);

		if(rds < 0) {

			/*
			 * Broken symlink
			 */

			write(out, "cd: The symbolic link is broken!\n", 34);
			return STATUS_CD_ERROR;

		}

		struct string * phys_dir_str = make_string(NULL);	

		append(phys_dir_str, pwd_str);
		append_cstr(phys_dir_str, buff);

		phys_dir_cstr = c_str(phys_dir_str);	

		free_string(phys_dir_str);
		free(phys_dir_str);
	}

	free_string(path_str);
	free_string(pwd_str);

	free_string(dir);	
	
	setenv("PWD", phys_dir_cstr, 1);

	return STATUS_CD_SUCCESS;
}
