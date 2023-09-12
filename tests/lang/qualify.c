#include "test.h"

void decl()
{
    const int a;
    const const const int b;
    volatile volatile const int c;
    volatile const volatile int d;
    volatile int e;
}

void _volatile()
{
    int volatile a = 10;
    a *= 10;
    assert(a == 100);
}

int _restrict(int* restrict a, int* restrict b)
{
    *a = 5;
    *b = 6;
    return (*a) * (*b);
}

int main()
{
    decl();
    _volatile();

    int a = 10;
    int b = 20;
    int c = _restrict(&a, &b);
    assert(a == 5);
    assert(b == 6);
    assert(c == 30);

    SUCCESS;
}
