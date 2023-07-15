#include "test.h"

void unary()
{
    int a = 10;
    int b = 20;
    int z = ~b; assert(z == -21);
    z = -b;     assert(z == -20);
    z= !b;      assert(z == 0);
    z = ~z;     assert(z == -1);
    z = -z;     assert(z == 1);
    z = !z;     assert(z == 0);
}

void unary2()
{
    int z = 0, b = 20;

    z = ~z + b; assert(z == 19);
    z = -z + b; assert(z == 1);
    z = !z + b; assert(z == 20);
}

void unary3()
{
    int a = 6, b = 5;
    int c = b++;  assert(c == 5);
    c = b++ + a;  assert(c == 12);
    c = b++ + 5;  assert(c == 12);
    c = a + b++;  assert(c == 14);
    c = 5 + b++;  assert(c == 14);
    c = ++b + a;  assert(c == 17);
    c = ++b + 10; assert(c == 22);
    c = 10 + ++b; assert(c == 23);
    c = a + ++b;  assert(c == 20);
}

void unary4()
{
    int a = 3, b = 14;

    int c = b--;    assert(c == 14);
    c = b-- + a;    assert(c == 16);
    c = b-- + 5;    assert(c == 17);
    c = a + b--;    assert(c == 14);
    c = 5 + b--;    assert(c == 15);
    c = --b + a;    assert(c == 11);
    c = --b + 10;   assert(c == 17);
    c = 10 + --b;   assert(c == 16);
    c = a + --b;    assert(c == 8);
}

int main()
{
    unary();
    unary2();
    unary3();
    unary4();

    SUCCESS;
}
