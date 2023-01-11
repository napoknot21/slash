#include "internals.h"
#include "slasherrno.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

	if (argc == 2) {
		src = argv[1];
		size_t scode;
		sscanf(src, "%ld", &scode);

		if (scode > MAX_EXIT_VAL) {

			/*
			 * Too big return value
			 */

			return STATUS_EXIT_ERROR;
		}

		code = (short) scode;
	}
	is_exit_call = 1;

	return code;
}
