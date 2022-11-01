#include <stdio.h>
#include "testlib.h"

int f() {
	return assert(1, __LINE__, __FILE__);
}
int g() {
	return assert(0, __LINE__, __FILE__);
}
int main() {
	printf("Test Preview\n");
	test(f, "f");
	test(g,"g");
	return 0;
}
