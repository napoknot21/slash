#include "slasherrno.h"
#include "errno.h"

short slasherrno = 0;
short is_exit_call = 0;


int of_errno() {
	switch (errno) {
		case EACCES: return S_EACCES;
		case ENOEXEC: return S_ENOCMD;
		case ENOENT: return S_ENOCMD;
		default:
			return errno;
	}
}
