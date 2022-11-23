#include "string.h"

#include <stdio.h>
#include <string.h>

struct string *make_string(const char *ch)
{
	struct string *str = malloc(sizeof(struct string));
	str->cnt = make_vector(sizeof(char), NULL);

	if(ch) {
		size_t ch_s = strlen(ch);

		reserve(str->cnt, ch_s * 2);
		memmove(str->cnt->data, ch, ch_s);

		str->cnt->size += ch_s;
	} else {

		reserve(str->cnt, 32);
	}

	return str;
}

void free_string(struct string *str)
{
	if(!str->cnt) return;
	free_vector(str->cnt);
	free(str);
}

void u_free_string(struct string *str) {
	free_vector(str->cnt);
}

void push_back_str(struct string *str, char c)
{
	push_back(str->cnt, &c);
}

void pop_back_str(struct string *str)
{
	pop_back(str->cnt);
}

void append(struct string *dst, struct string *src)
{
	size_t src_s = src->cnt->size;
	size_t dst_s = dst->cnt->size;

	size_t ncap = dst->cnt->capacity;

	while (dst_s + src_s >= ncap) {
		ncap *= 2;
	}

	reserve(dst->cnt, ncap);

	memmove((void *)((char *)(dst->cnt->data + dst_s)), src->cnt->data,
		src_s);
	dst->cnt->size += src_s;
}

void append_cstr(struct string * dst, const char * src)
{
	struct string * tstr = make_string(src);
	append(dst, tstr);

	free_string(tstr);
}

void clear_str(struct string *str)
{
	clear(str->cnt);
}

char *at_str(struct string *str, size_t pos)
{
	return (char *)at(str->cnt, pos);
}

char *front_str(struct string *str)
{
	return (char *)front(str->cnt);
}

char *back_str(struct string *str)
{
	return (char *)back(str->cnt);
}

struct string *substr(struct string *str, size_t from, size_t to)
{
	size_t str_s = str->cnt->size;

	if (from >= str_s || to <= from) {
		return NULL;
	}

	to = to > str_s ? str_s : to;
	char *sub = malloc(to - from + 1);

	sub[to - from] = 0x0;

	memmove(sub, (void *)((char *)(str->cnt->data + from)), to - from);
	struct string *substr = make_string(sub);

	free(sub);

	return substr;
}

size_t size_str(struct string *str)
{
	return str->cnt->size;
}

int cmp_str(struct string *str_a, struct string *str_b)
{
	if (size_str(str_a) != size_str(str_b)) return 1;

	for (size_t k = 0; k < str_a->cnt->size; k++) {
		if (*at_str(str_a, k) != *at_str(str_b, k)) {
			return 1;
		}
	}

	return 0;
}

char *c_str(struct string *str)
{
	if(!str) return NULL;

	size_t size = size_str(str);
	char * data = malloc(size + 1);

	*(data + size) = 0x0;

	for(size_t k = 0; k < size; k++) {
		data[k] = *at_str(str, k);
	}

	return (char *) data;
}

int empty_str(struct string *str)
{
	return !str ? 0 : size_str(str);
}

struct vector * split_str(struct string *str, char sep)
{
	size_t k = 0, beg = 0;
	size_t str_s = size_str(str);

	struct vector * svec = make_vector(sizeof(struct string), (void (*)(void*)) u_free_string);

	for(; k < str_s; k++) {

		char curr = *at_str(str, k);

		if(curr == sep || k + 1 == str_s) {

			struct string * sub = substr(str, beg, k);
			if(empty_str(sub)) push_back(svec, sub);
			free(sub);

			beg = k + 1;
		}

	}	

	return svec;
}

struct string * bind_str(struct vector *vec, char sep)
{
	char base[] = {sep};

	struct string * res = make_string(base);
	for(size_t i = 0; i < vec->size; i++) {

		append(res, at(vec, i));
		if(i + 1 < vec->size) push_back_str(res, sep);

	}

	return res;
}
