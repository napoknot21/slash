#include "internals.h"
#include "slasherrno.h"

#include <stdio.h>
#include <unistd.h>

int builtin_exit(int in, int out, int err, int argc, char **argv)
{
	if (argc > 2) {

		/*
		 * Too much arguments, print man
		 */

		return STATUS_EXIT_ERROR;
	}

	short code = slasherrno;

	char *src = NULL;

	if (in != STDIN_FILENO) {
		char buf[] = {0, 0, 0, 0, 0};
		read(in, buf, 4);
		src = buf;
	} else if (argc == 2) {
		src = argv[1];
	}
	if (src) {
		size_t scode;
		sscanf(argv[1], "%ld", &scode);

		if (scode > MAX_EXIT_VAL) {

			/*
			 * Too big return value
			 */

			return STATUS_EXIT_ERROR;
		}

		code = (short)scode;
	}
	is_exit_call = 1;

	return code;
}
