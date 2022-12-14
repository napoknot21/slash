#include "path.h"
#include <string.h>
#include <stdlib.h>

struct string * normalize_path(struct string * path, struct string * wd, int symlnk_check)
{
	if(!empty_str(path))
		return copy_str(wd);

	char * fs = front_str(path);
	int is_absolute = *fs == '/';

	struct string * complete = make_string(NULL);

	if(is_absolute) {

		/*
		 * This is an absolute path
		 */
		
		append(complete, path);	
	}

	/*
	 * This is a relative path to working directory
	 */

	append(complete, wd);
	push_back_str(complete, '/');
	append(complete, path);	

	if(*back_str(complete) != '/') 
		push_back_str(complete, '/');

	struct vector * complete_split = split_str(complete, '/');
	struct vector * fragments = make_vector(sizeof(struct string), destruct_string, copy_str);

	/*
	 * Comparators
	 */

	struct string * back_up_str = make_string(".."),
		      * curr_str = make_string(".");

	/*
	 * For each .. in path_split we, pop in wd_split
	 */

	for(size_t i = 0; i < complete_split->size; i++) {

		struct string * pith = at(path_split, i);
	
		if(symlnk_check) {
		
			struct string * current_pwd = bind_str(fragments, '/');
			char * buffer = malloc(PHYSICAL_PATH_BUFFER);

			const char * cp_path = c_str(current_pwd);
			ssize_t si = readlink(cp_path, buffer, PHYSICAL_PATH_BUFFER);
			
			struct string * current_path = make_string(buffer);

			free(cp_path);
			free(buffer);

			if(si) 
				normalize_path(current_path, current_pwd, 0);

			free_string(current_path);
			free_string(current_pwd);
		}

		if(!cmp_str(pith, curr_str))
			continue;

		if(!cmp_str(pith, back_up_str))
			pop_back(fragments);
		else
			push_back(fragments, pith);	
	}

	free_string(back_up_str);
	free_string(curr_str);

	free_vector(complete_split);

	/*
	 * Binding of the new wd path
	 */

	struct string * res = bind_str(fragments, '/');

	return res;
}
