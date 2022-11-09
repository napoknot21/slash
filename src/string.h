#ifndef STRING_H
#define STRING_H

typedef struct string
{
	vector * cnt;
} string;

string * make_string();
void free_string(string *);

void append(string *, char);

#endif 
