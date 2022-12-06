#include "proc.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

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

int built_out(int out, int err, int argc, char ** argv)
{
	if(argc < 0) 
		return 1;
	
	char ** exargv = exec_format(argc, argv);

	/*
	 * Lancement du processus
	 */

	pid_t process = fork();

	switch(process) {

	case -1: 
		return 1;

	case 0:
		/*
		 * Changements des fd de sortie
		 */

		dup2(out, STDOUT_FILENO);
		dup2(err, STDERR_FILENO);

		/*
		 * Recouvrement du processus
		 */

		int status = execvp(argv[0], exargv);

		if(status == -1) {

			dprintf(err, "%s: erreur à l'éxecution!\n", argv[0]);	

		}

		break;

	default:
		wait(NULL);
		free(exargv);
	}

	return 0;
}
