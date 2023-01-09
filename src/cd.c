#include "internals.h"
#include "path.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CONSISTENT_CHECK_SUCCESS 0
#define CONSISTENT_CHECK_NOEXISTS 1
#define CONSISTENT_CHECK_NODIR 2

ssize_t get_physical_path(char * dst, const char * path, size_t size)
{
	memset(dst, 0x0, size);
	return readlink(path, dst, size);
}

int check_path_consistency(struct string * path)
{
	char * cpath = c_str(path);
	struct stat dirstat;

	int stat_s = lstat(cpath, &dirstat);
	free(cpath);

	if(stat_s == -1)
		return CONSISTENT_CHECK_NOEXISTS;

	mode_t dirmode = dirstat.st_mode;

	int symlink = S_ISLNK(dirmode);
	int dir = S_ISDIR(dirmode);

	if(!symlink && !dir)
		return CONSISTENT_CHECK_NODIR;

	return CONSISTENT_CHECK_SUCCESS;
}

int builtin_cd_back(int in, int out, int err, const char * pwd)
{
	char * old = getenv("OLDPWD");

	if(!old) {
		dprintf(err, "cd: OLDPWD isn't set!\n");
		return STATUS_CD_ERROR;
	}

	setenv("PWD", old, 1);
	setenv("OLDPWD", pwd, 1);

	return STATUS_CD_SUCCESS;
}

int builtin_cd_kind(int in, int out, int err, struct string * pwd, struct string * path, int kind)
{
	struct string * norm = normalize_path(path, pwd, kind);
	int status = check_path_consistency(norm);

	if(status == CONSISTENT_CHECK_SUCCESS) {

		char * cnorm = c_str(norm),
		     * cpwd  = c_str(pwd);

		status = chdir(cnorm);

		if(!status) {

			setenv("OLDPWD", cpwd, 1);
			setenv("PWD", cnorm, 1);

		}

		free(cnorm);
		free(cpwd);
	}

	free_string(norm);

	return status;
}

int builtin_cd(int in, int out, int err, int argc, char ** argv)
{
	int kind = LOGICAL_PATH;

	/*
	 * This command admits the following options :
	 * 	- logical path (-L) [default]
	 * 	- physical path (-P)
	 */

	const char * path = getenv("HOME"), * pwd = getenv("PWD");

	for (int k = 1; k < argc; k++) {

		if (!strcmp(argv[k], "-L"))
			continue;
		else if (!strcmp(argv[k], "-P"))
			kind = PHYSICAL_PATH;
		else {
			path = argv[k];
			break;
		}
	}

	if(!strcmp(path, "-"))
		return builtin_cd_back(in, out, err, pwd);

	struct string * path_str = make_string(path),
		      * pwd_str = make_string(pwd);

	int status = 0;
	if(kind == LOGICAL_PATH)
		status = builtin_cd_kind(in, out, err, pwd_str, path_str, 0);

	if(kind == PHYSICAL_PATH || status)
		status = builtin_cd_kind(in, out, err, pwd_str, path_str, 1);

	free_string(path_str);
	free_string(pwd_str);

	return status;
}
