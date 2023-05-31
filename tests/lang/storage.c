#include "tests/test.h"

// auto, typedef and specifiers about linking are
// tested in auto.c, typedef.c, link1.c and link2.c.

void _register()
{
    // The extent to which such suggestions are
    // effective is implementation-defined. (6.7.1[8])
    // And you must have understood what I mean.
    register int a = 10;
    a *= 10;
    assert(a == 100);

    register char b[8];
    b[0] = 30;
    assert(b[0] == 30);
}

int regfunc(int register a, int register b)
{
    return a * b;
}

void _constexpr()
{
    int constexpr a = 20;
    assert(a == 20);

    int constexpr b = a + 5;
    assert(b == 25);

    int* pa = &a;
    assert(*pa == 20);
}

int _static(int i, int val)
{
    static int array[20];
    if (val == 0)
        return array[i];
    array[i] = val;
    return 0;
}

int main()
{
    _register();
    assert(regfunc(5, 6) == 30);

    _static(2, 30);
    assert(_static(2, 0) == 30);
}
