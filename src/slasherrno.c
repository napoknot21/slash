#include "slasherrno.h"

#include "errno.h"

#include <string.h>
#include <unistd.h>

short slasherrno = 0;
short is_exit_call = 0;

#define BUF_SIZE 64
char buf[BUF_SIZE];

int of_errno()
{
	switch (errno) {
	case EACCES:
		return S_EACCES;
	case ENOEXEC:
		return S_ENOCMD;
	case ENOENT:
		return S_ENOCMD;
	default:
		return errno;
	}
}

static char *get(int code)
{
	char *s;
	memset(buf, 0, BUF_SIZE);
	switch (code) {
	case S_EACCES:
		return strerror(EACCES);
	case S_ENOCMD:
		return strerror(ENOEXEC);
	case S_ESYNTAX:
		s = "-slash : A syntax error has occured";
		break;
	case S_EUNKNOWN:
		s = "-slash : An unexpected token has been found";
		break;
	case S_EFAIL:
	default:
		s = "-slash : Something went wrong !";
	}
	memcpy(buf, s, strlen(s) + 1);
	return buf;
}

void werror() {
	get_error();
	size_t len = strlen(buf);
	memcpy(buf + len, "\n", 2);
	write(STDERR_FILENO, buf, len + 2);
}

char *get_error() {
	return get(slasherrno);
}


