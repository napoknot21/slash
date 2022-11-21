#include "path.h"
#include <string.h>
#include <stdio.h>

struct string * normalize_path(struct string * path, struct string * wd)
{
	if(!empty_str(path)) {
		return wd;
	}

	char * fs = front_str(path);	

	if(*fs == '/') {

		/*
		 * This is an absolute path
		 */

		return path;

	}

	/*
	 * This is a relative path to working directory
	 */	

	push_back_str(wd, '/');

	if(*back_str(path) != '/') push_back_str(path, '/');

	struct vector * wd_split = split_str(wd, '/');
	struct vector * path_split = split_str(path, '/');

	struct string * back_up_str = make_string(".."),
		      * curr_str = make_string(".");

	/*
	 * At each .. in path_split we, pop in wd_split
	 */	

	for(size_t i = 0; i < path_split->size; i++) {

		struct string * pith = at(path_split, i);	
		if(!cmp_str(pith, curr_str)) continue;

		if(!cmp_str(pith, back_up_str)) {
	
			pop_back(wd_split);

		} else {

			push_back(wd_split, pith);

		}

	}

	free(path_split);

	free_string(back_up_str);
	free_string(curr_str);

	/*
	 * Binding of the new wd path
	 */

	struct string * res = make_string("/");

	for(size_t i = 0; i < wd_split->size; i++) {

		append(res, at(wd_split, i));
		if(i + 1 < wd_split->size) push_back_str(res, '/');

	}

	free(wd_split);

	return res;
}
