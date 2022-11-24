#ifndef PATH_H
#define PATH_H

#include "string.h"

#define PHYSICAL_PATH_BUFFER 512
#define LOGICAL_PATH 0
#define PHYSICAL_PATH 1

//extern const char * lastwd;
struct string *normalize_path(struct string *, struct string *);

#endif
