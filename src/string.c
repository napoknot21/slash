#include "string.h"
#include <string.h>
#include <stdio.h>

string * make_string(const char * ch)
{	
	string * str = malloc(sizeof(struct string));
	str->cnt = make_vector(sizeof(char), NULL);

	if(ch)
	{
		size_t ch_s = strlen(ch);	

		reserve(str->cnt, ch_s * 2);	
		memmove(str->cnt->data, ch, ch_s);

		str->cnt->size += ch_s;
	}

	return str;
}

void free_string(string * str)
{
	free_vector(str->cnt);
	free(str);
}

void push_back_str(string * str, char c)
{
	push_back(str->cnt, &c);
}

void pop_back_str(string * str)
{
	pop_back(str->cnt);
}

void append(string * dst, string * src)
{
	size_t src_s = src->cnt->size;
	size_t dst_s = dst->cnt->size;

	size_t ncap = dst->cnt->capacity;	

	while(dst_s + src_s >= ncap)
	{
		ncap *= 2;
	}

	reserve(dst->cnt, ncap);		

	memmove((void*)((char*) (dst->cnt->data + dst_s)), src->cnt->data, src_s);
	dst->cnt->size += src_s;
}

void clear_str(string * str)
{
	clear(str->cnt);
}

char * at_str(string * str, size_t pos)
{
	return (char*) at(str->cnt, pos);
}

char * front_str(string * str)
{
	return (char*) front(str->cnt);
}

char * back_str(string * str)
{
	return (char*) back(str->cnt);
}

string * substr(string * str, size_t from, size_t to)
{
	size_t str_s = str->cnt->size;

	if(from >= str_s || to <= from)
	{
		return NULL;
	}

	to = to > str_s ? str_s : to;
	char * sub = malloc(to - from);

	memmove(sub, (void*)((char*)(str->cnt->data + from)), to - from);
	
	return make_string(sub);
}

size_t size_str(string * str)
{
	return str->cnt->size;
}
