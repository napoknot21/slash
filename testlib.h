#ifndef SLASH_TESTLIB_H
#define SLASH_TESTLIB_H

#define ASSERT(b) (assert((b), __LINE__, __FILE__))

/**
 * Do the Test given in argument and print informations about it, like the execution time.
 *
 * @param f The function that will be tested.
 * @param name The name of the test.
 */
void test(int (*f)(), const char *name);

/**
 * Similar to assert from assert.h. Do the assertion of the given formula. If the formula is false, it will print an error message with the line and file.
 *
 * @param b the formula that will be asserted
 * @param line The line where the function was called (used for print)
 * @param filename The file name where the function was called (used for print) 
 */
int assert(int b, int line, const char *filename);

#endif
