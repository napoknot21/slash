#include <stdio.h>
#include "testlib.h"

int f() 
{
	return ASSERT(1);
}
int g() 
{
	return ASSERT(0);
}
int main() 
{
	int val = 1;
	printf("Test Preview\n");
	val &= test(f, "f");
	//val &= test(g,"g");
	return val ? 0 : 1;
}
