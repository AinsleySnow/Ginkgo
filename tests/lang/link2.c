#include "test.h"

static int b = 20;

void scope()
{
    // 6.2.2[4]
    // if the prior declaration specifies internal or external linkage,
    // the linkage of the identifier at the later declaration is the same
    // as the linkage specified at the prior declaration. If no prior
    // declaration is visible, or if the prior declaration specifies no
    // linkage, then the identifier has external linkage.
    extern int b;
    assert(b == 20);
}

int main()
{
    extern int d;
    extern int e;
    int func2(int);

    assert(d == 40);
    assert(e == 50);
    assert(func2(2) == 5);
}
