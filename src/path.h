#ifndef PATH_H
#define PATH_H

#include "string.h"

#define LOGICAL_PATH 0
#define PHYSICAL_PATH 1

struct string *normalize_path(struct string *, struct string *);

#endif
