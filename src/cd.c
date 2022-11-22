#include "internals.h"
#include "path.h"
#include "slasherrno.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

ssize_t physical_path(char * dst, size_t size, const char * path)
{
	memset(dst, 0x0, size);
	return readlink(path, dst, size);
}

int builtin_cd(int in, int out, int argc, char **argv)
{
	const char *path = getenv("HOME"), *pwd = getenv("PWD");
	
	if(!lastwd) lastwd = path;

	char ppwd[PHYSICAL_PATH_BUFFER];
	ssize_t rds = physical_path(ppwd, PHYSICAL_PATH_BUFFER, pwd);

	int kind = LOGICAL_PATH;

	/*
	 * This command admits the following options :
	 * 	- logical path (-L) [default]
	 * 	- physical path (-P)
	 */

	for (int k = 1; k < argc; k++) {

		if (!strcmp(argv[k], "-L")) {

			continue;

		} else if (!strcmp(argv[k], "-P")) {

			kind = PHYSICAL_PATH;

		} else {
			path = argv[k];
			break;
		}
	}

	if(!strcmp(path, "-")) {

		setenv("PWD", lastwd, 1);
		return STATUS_CD_SUCCESS;

	}

	struct string *path_str = make_string(path),
		      *pwd_str = make_string(pwd);

	struct string *dir = normalize_path(path_str, pwd_str);

	/*
	 * We check whether that directory exists,
	 * or is a symlink
	 */

	char *dir_cstr = c_str(dir);

	struct stat dirstat;
	int stat_s = lstat(dir_cstr, &dirstat);

	/*
	 * Failed to retrieve informations on the node
	 */

	if (stat_s == -1 && rds > 0) {
	
		free_string(dir);	
		free(dir_cstr);

		struct string * ppwd_str = make_string(ppwd);
		struct vector * spl_pwd = split_str(pwd_str, '/');
		
		pop_back(spl_pwd);
		push_back(spl_pwd, ppwd_str);	

		struct string * rpwd_str = bind_str(spl_pwd, '/');	
	
		free_string(ppwd_str);

		dir = normalize_path(path_str, rpwd_str);

		dir_cstr = c_str(dir);
		stat_s = lstat(dir_cstr, &dirstat);

		free(spl_pwd);

	}

	if(stat_s == -1) {
	
		write(out, "cd: That directory does not exist!\n", 36);
		return STATUS_CD_ERROR;
	
	}

	mode_t dirmode = dirstat.st_mode;

	/*
	 * Not a directory
	 */

	int symlink = S_ISLNK(dirmode);

	if (!S_ISDIR(dirmode) && !symlink) {

		write(out, "cd: Not a directory!\n", 22);
		return STATUS_CD_ERROR;
	}

	const char *phys_dir_cstr = dir_cstr;
	if (symlink && kind == PHYSICAL_PATH) {

		/*
		 * We seek the physical path
		 */

		char buff[PHYSICAL_PATH_BUFFER];
		rds = physical_path(buff, PHYSICAL_PATH_BUFFER, dir_cstr);

		if (rds < 0) {

			/*
			 * Broken symlink
			 */

			write(out, "cd: The symbolic link is broken!\n", 34);
			return STATUS_CD_ERROR;
		}

		struct string *phys_dir_str = make_string(NULL);

		append(phys_dir_str, pwd_str);
		append_cstr(phys_dir_str, buff);

		phys_dir_cstr = c_str(phys_dir_str);	

		free_string(phys_dir_str);	
	}

	free_string(path_str);
	free_string(pwd_str);

	free_string(dir);

	setenv("PWD", phys_dir_cstr, 1);	

	return STATUS_CD_SUCCESS;
}
