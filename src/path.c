#include "path.h"
#include <string.h>
#include <stdlib.h>

const char * lastwd = NULL;

struct string * normalize_path(struct string * path, struct string * wd)
{
	if(!empty_str(path)) {
		char *tmp = c_str(wd);
		struct string *ret = make_string(tmp);
		free(tmp);
		return ret;
	}

	char * fs = front_str(path);

	if(*fs == '/') {

		/*
		 * This is an absolute path
		 */
		char *tmp = c_str(path);
		struct string *ret =  make_string(tmp);
		free(tmp);
		return ret;

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
		if(!cmp_str(pith, curr_str)) {
			u_free_string(pith);
			continue;
		}

		if(!cmp_str(pith, back_up_str)) {

			free_string(pop_back(wd_split));
			u_free_string(pith);

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

	struct string * res = bind_str(wd_split, '/');

	free_vector(wd_split);
	return res;
}
