#include "internal.h"

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
	const char * cpath = c_str(path);
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

void builtin_cd_back(int out, int err, const char * pwd)
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

void builtin_cd_physical(int out, int err, const char * pwd, const char * path)
{
	struct string * pwd_str = make_string(pwd);
	struct string * path_str = make_string(path);
	
	struct string * complete_path = make_string(NULL);

	if(front_str(path_str) == '/') {

		/*
		 * Ignore the pwd
		 */

		append(complete_path, path_str);
		free_string(path_str);
	
	} else {

		append(complete_path, pwd_str);
		push_back_str(complete_path, '/');
		append(complete_path, path_str);

		free_string(pwd_str);
		free_string(path_str);

	}

	free_string(complete_path);	

	struct vector * part = split_str(complete_path, '/');
	struct string * progressive = make_string(NULL);

	int status;

	for(size_t i = 0; i < part->size; i++) {

		push_back_str(progressive, '/');
		append(progressive, at(part, i));

		const char * cpath = c_str(progressive);
		char * ppBuffer = malloc(PHYSICAL_PATH_BUFFER);

		ssize_t rd_size = readlink(cpath, ppBuffer, PHYSICAL_PATH_BUFFER);
		free(cpath);
		
		if(rd_size) {
			push_back_str(progressive, '/');
			append_cstr(progressive, ppBuffer);
		}

		free(ppBuffer);
		status = check_path_consistency(progressive);

		if(status != CONSISTENT_CHECK_SUCCESS) break;	
	}

	free_vector(part);

	if(status)
		return -1;

	setenv("OLDPWD", pwd, 1);
	setenv("PWD", progressive, 1);

	return 0;
}

int builtin_cd(int out, int err, int argc, char ** argv)
{
	int kind = LOGICAL_PATH;	

	/*
	 * This command admits the following options :
	 * 	- logical path (-L) [default]
	 * 	- physical path (-P)
	 */

	const char * path = getenv("HOME"), * pwd = getenv("PWD");
	char * physical_pwd[PHYSICAL_PATH_BUFFER];

	ssize_t ppwd_size = get_physical_path(ppwd, PHYSICAL_PATH_BUFFER, pwd);

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
		return builtin_cd_back(in, err, pwd);

	if(kind == PHYSICAL_PATH)
		return builtin_cd_physical(in, err, physical_pwd);

	return 0;
}
