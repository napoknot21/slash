#include "proc.h"
#include "slasherrno.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
/*
void extern_handler(int sig)
{
	printf("%d\n", sig);
	switch(sig) {

		case SIGINT: break;

		case SIGTERM: break;

	}
}
*/
char ** exec_format(int argc, char ** argv)
{
	/*
	 * Reformatage du pointeur, avec NULL en fin
	 */

	size_t exs = (argc + 1) * sizeof(char*);
	char ** ref = malloc(exs);

	for(int i = 0; i < argc; i++) {

		ref[i] = argv[i];

	}

	ref[argc] = NULL;

	return ref;
}

int built_out(int in, int out, int err, int argc, char ** argv)
{
	if(argc <= 0)
		return 1;

	char ** exargv = exec_format(argc, argv);

	dup2(in, STDIN_FILENO);
	dup2(out, STDOUT_FILENO);
	dup2(err, STDERR_FILENO);

	/*
	 * Recouvrement du processus
	 */

	execvp(argv[0], exargv);	
		
	perror(argv[0]);
	free(exargv);
	_exit(of_errno());

}
