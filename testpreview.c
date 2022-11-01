#include <stdio.h>
#include "testlib.h"

int f() {
	return ASSERT(1);
}
int g() {
	return ASSERT(0);
}
int main() {
	printf("Test Preview\n");
	test(f, "f");
	test(g,"g");
	return 0;
}
