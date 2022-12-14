#include "path.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct string * normalize_path(struct string * path, struct string * wd, int symlnk_check)
{
	if(!empty_str(path)) {
		
		struct string * copy = malloc(sizeof(struct string));
		copy_str(wd, copy);
		
		return copy;
	
	}

	char * fs = front_str(path);
	int is_absolute = *fs == '/';

	struct string * complete = make_string(NULL);

	if(is_absolute) {

		/*
		 * This is an absolute path
		 */
		
		append(complete, path);	
	
	} else {

		append(complete, wd);
		push_back_str(complete, '/');
		append(complete, path);

	}

	/*
	 * This is a relative path to working directory
	 */

	if(*back_str(complete) != '/') 
		push_back_str(complete, '/');	

	struct vector * complete_split = split_str(complete, '/');
	struct vector * fragments = make_vector(
			sizeof(struct string), 
			(void (*)(void*)) destruct_string, 
			(void (*)(void*, void*)) copy_str);

	free_string(complete);	

	/*
	 * Comparators
	 */

	struct string * back_up_str = make_string(".."),
		      * curr_str = make_string(".");

	/*
	 * For each .. in path_split we, pop in wd_split
	 */

	for(size_t i = 0; i < complete_split->size; i++) {

		struct string * pith = at(complete_split, i);

		if(!cmp_str(pith, curr_str))
			continue;

		if(!cmp_str(pith, back_up_str))
			pop_back(fragments);
		else
			push_back(fragments, pith);	

		if(symlnk_check) {
		
			struct string * current_pwd = bind_str(fragments, '/');
			char * buffer = malloc(PHYSICAL_PATH_BUFFER);

			memset(buffer, 0x0, PHYSICAL_PATH_BUFFER);

			char * cp_path = c_str(current_pwd);
			ssize_t si = readlink(cp_path, buffer, PHYSICAL_PATH_BUFFER);

			free(cp_path);

			if(si > 0) {	

				pop_back(fragments);
				free_string(current_pwd);

				current_pwd = bind_str(fragments, '/');

				struct string * current_path = make_string(buffer);	
				struct string * n = normalize_path(current_path, current_pwd, 0);

				push_back_str(n, '/');

				free_vector(fragments);
				fragments = split_str(n, '/');

				free_string(n);
				free_string(current_path);
			}

			free(buffer);
			free_string(current_pwd);	
		}

	}

	free_string(back_up_str);
	free_string(curr_str);
	
	free_vector(complete_split);

	/*
	 * Binding of the new wd path
	 */

	struct string * res = bind_str(fragments, '/');
	free_vector(fragments);

	return res;
}
