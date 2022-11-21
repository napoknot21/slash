#include "internals.h"
#include "slasherrno.h"
#include <stdio.h>

int builtin_exit(int in, int out, int argc, char ** argv)
{
	if(argc > 2) {

		/*
		 * Too much arguments, print man
		 */

		return STATUS_EXIT_ERROR;
	}

	short code = slasherrno;

	if(argc == 2) {

		size_t scode;
		sscanf(argv[1], "%ld", &scode);

		if(scode > MAX_EXIT_VAL) {

			/*
			 * Too big return value
			 */

			return STATUS_EXIT_ERROR;
		}

		code = (short) scode;

	}

	return code;
}
