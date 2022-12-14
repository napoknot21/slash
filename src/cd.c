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

int builtin_cd(int out, int err, int argc, char **argv)
{
	const char *path = getenv("HOME"), *pwd = getenv("PWD");	

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
		char *oldpwd = getenv("OLDPWD");
		if (oldpwd == NULL) {
			dprintf(err, "cd: OLDPWD not set\n");
			return STATUS_CD_ERROR;
		}
		setenv("PWD", oldpwd, 1);
		setenv("OLDPWD", pwd, 1);
		return STATUS_CD_SUCCESS;

	}

	struct string *path_str = make_string(path),
		      *pwd_str = make_string(pwd),
		      *ppwd_str = make_string(ppwd), //indirect loss
		      *rpwd_str = NULL;

	size_t ppwd_str_s = size_str(ppwd_str);

	if(ppwd_str_s > 0) {

		struct vector * spl_pwd = split_str(pwd_str, '/');

		push_back(spl_pwd, ppwd_str);
		rpwd_str = bind_str(spl_pwd, '/');
		free_string(ppwd_str);
		ppwd_str = NULL;

		if(kind == PHYSICAL_PATH) {

			free_string(pwd_str);
			pwd_str = rpwd_str;
			rpwd_str = NULL;

		}
		free_vector(spl_pwd);
		//free_vector(spl_pwd); //MEMLEAK
	}

	if (ppwd_str != NULL) {
		free_string(ppwd_str);
		ppwd_str = NULL;
	}

	struct string *dir = normalize_path(path_str, pwd_str); //memleak

	/*
	 * We check whether that directory exists,
	 * or is a symlink
	 */

	char *dir_cstr = c_str(dir);
	free_string(dir);

	struct stat dirstat;
	int stat_s = lstat(dir_cstr, &dirstat);

	/*
	 * Failed to retrieve informations on the node
	 */

	if(stat_s == -1 && rds > 0 && ppwd_str_s > 0) {

		free(dir_cstr);

		if (rpwd_str != NULL) {
			free_string(pwd_str);
			pwd_str = rpwd_str;
			rpwd_str = NULL;
		}

		dir_cstr = c_str(pwd_str);

		stat_s = lstat(dir_cstr, &dirstat);
	}

	if(stat_s == -1) {

		dprintf(out, "cd: That directory does not exist!\n");
		goto error;

	}

	mode_t dirmode = dirstat.st_mode;

	/*
	 * Not a directory
	 */

	int symlink = S_ISLNK(dirmode);

	if (!S_ISDIR(dirmode) && !symlink) {

		dprintf(out, "cd: Not a directory!\n");
		goto error;

	}

	char *phys_dir_cstr = dir_cstr;
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

			dprintf(out, "cd: The symbolic link is broken!\n");
			goto error;
		}

		struct string *phys_dir_str = make_string(NULL);

		append(phys_dir_str, pwd_str);
		append_cstr(phys_dir_str, buff);

		free(dir_cstr);
		phys_dir_cstr = c_str(phys_dir_str);

		free_string(phys_dir_str);
	}
	if (rpwd_str != NULL)
			free_string(rpwd_str);

	free_string(path_str);
	free_string(pwd_str);

	setenv("OLDPWD",pwd,1);
	setenv("PWD", phys_dir_cstr, 1);

	free(phys_dir_cstr);

	return STATUS_CD_SUCCESS;

	error:
		free_string(path_str);
		free(dir_cstr);
		free_string(pwd_str);
		if (ppwd_str != NULL)
			free_string(ppwd_str);
		if (rpwd_str != NULL)
			free_string(rpwd_str);

		return STATUS_CD_ERROR;

}
