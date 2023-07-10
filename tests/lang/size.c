#include "test.h"

void common()
{
    assert(sizeof(char) == 1);
    assert(sizeof(short) == 2);
    assert(sizeof(int) == 4);
    assert(sizeof(long) == 8);

    assert(sizeof(float) == 4);
    assert(sizeof(double) == 8);
    // assert(sizeof(long double) == 16);
}

void array()
{
    assert(sizeof(int [5]) == 20);
    assert(sizeof(int* [5]) == 40);
}

void pointer()
{
    assert(sizeof(int*) == 8);
    assert(sizeof(int (*)[5]) == 8);
    assert(sizeof(int (*)(int)) == 8);
    assert(sizeof(int (*(*) [5])(int)) == 8);
}

void _enum()
{
    enum a : long { A_A, A_B, A_C };
    enum b : int  { B_A, B_B, B_C };
    enum c        { C_A, C_B, C_C };
    enum a aa = A_A;
    enum b bb = B_A;
    enum c cc = C_A;
    assert(sizeof(enum a) == 8);
    assert(sizeof(enum b) == 4);
    assert(sizeof(enum c) == 4);
    assert(sizeof(aa) == 8);
    assert(sizeof(bb) == 4);
    assert(sizeof(cc) == 4);
}

void expr()
{
    assert(sizeof(1 + 1) == 4);

    int a = 10;
    long b = 20;
    assert(sizeof(a + b) == 8);
    assert(sizeof(a++) == 4);
    assert(a == 10);

    float c = 2.5;
    assert(sizeof(b * c) == 4);
}

void var()
{
    int a = 20;
    assert(sizeof(a) == 4);
    int b[30];
    assert(sizeof(b) == 120);
}

int main(void)
{
    common();
    array();
    pointer();
    _enum();
    expr();
    var();

    SUCCESS;
}
