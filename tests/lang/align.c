#include "test.h"

void common()
{
    assert(alignof(char) == 1);
    assert(alignof(short) == 2);
    assert(alignof(int) == 4);
    assert(alignof(long) == 8);

    assert(alignof(float) == 4);
    assert(alignof(double) == 8);
    // assert(alignof(long double) == 16);

    assert(_Alignof(char) == 1);
    assert(_Alignof(short) == 2);
    assert(_Alignof(int) == 4);
    assert(_Alignof(long) == 8);

    assert(_Alignof(float) == 4);
    assert(_Alignof(double) == 8);
    // assert(_Alignof(long double) == 16);
}

void array()
{
    // When applied to an array type, the result is
    // the alignment requirement of the element type.
    // (6.5.3.4[3])
    assert(alignof(int [5]) == 4);
    assert(alignof(int* [5]) == 8);
}

void as()
{
    int alignas(8) a = 10;
    assert(alignof(a) == 8);

    int alignas(double) b = 30;
    assert(alignof(b) == 8);

    // When multiple alignment specifiers occur in a
    // declaration, the effective alignment requirement
    // is the strictest specified alignment.
    // (6.7.5[7])
    int alignas(4) alignas(8) alignas(16) c = 40;
    assert(alignof(c) == 16);

    int alignas(8) d[20];
    assert(alignof(d) == 8);
}

void enumas()
{
    enum a { A_A, A_B, A_C };
    enum a alignas(8) aa = A_A;
    assert(alignof(aa) == 8);
}

int main()
{
    common();
    array();
    as();
    enumas();

    SUCCESS;
}
