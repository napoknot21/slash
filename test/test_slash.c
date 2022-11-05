#include "test_token.h"
#include "testlib.h"

int main()
{
    int bool = 1;
    bool &= test(test_token, "token_new");
    return (bool) ? 0 : 1;
}