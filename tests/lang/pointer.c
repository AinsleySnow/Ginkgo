#include "test.h"

void multi()
{
    int va = 10;
    int* a = &va;
    assert(*a == 10);

    char vb = 'x';
    char* b = &vb;
    assert(*b == 'x');

    char**** c = 0;
    assert(c == 0);

    char vd = 'y';
    char* pd = &vd;
    char** ppd = &pd;
    char* const * const * d = &ppd;
    assert(***d == 'y');

    char ve = 'z';
    char* pe = &ve;
    char* const restrict * e = &pe;
    assert(**e == 'z');

    char vg = 'w';
    const char* restrict g = &vg;
    assert(*g == 'w');
}

void change()
{
    int a = 20;
    int* pa = &a;
    *pa = 40;
    assert(a == 40);
}

void arithm1()
{
    long a = 0x1234567890;
    char* p = &a;
    p += 2;
    assert(*p == 0x56);
    p -= 1;
    assert(*p == 0x78);
    p = p + 3;
    assert(*p == 0x12);
    p = p - 3;
    assert(*p == 0x78);
}

void arithm2()
{
    long a[5];
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    a[4] = 4;

    long* p = a;
    p += 2;     assert(*p == 2);
    p -= 1;     assert(*p == 1);
    p = p + 3;  assert(*p == 4);
    p = p - 3;  assert(*p == 1);
}

void arithm3()
{
    long a[5];
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    a[4] = 4;

    long* pa = a;
    long* pb = a + 2;
    assert(pb - pa == 2);
}

void bracket()
{
    long a[5];
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    a[4] = 4;

    long* p = a;
    long* pp = &(*(p + 2));

    p[2] = 20;          assert(a[2] == 20);
    p[3] = 40;          assert(a[3] == 40);
    *(p + 2) = 10;      assert(a[2] == 10);
    (p + 1)[2] = 60;    assert(a[3] == 60);
}

void infunc(int* a, int* b)
{
    *a = 30;
    *b = *a + 20;
}

int func()
{
    return 0;
}

int func2(int a, int b, int c)
{
    return a + b + c;
}

int (*func3(int a, int b, int c))()
{
    int d = a + b + c;
    assert(d == 6);
    return func;
}

int (*func4(int a, int b, int c))(int, int, int)
{
    int (*pfunc)(int, int, int) = func2;
    int d = pfunc(a, b, c);
    assert(d == 60);
    return pfunc;
}

int (*func5(int a, int b, int c))(int a, int b, int c)
{
    int (*pfunc)(int, int, int) = func2;
    return pfunc;
}

int (*(*func6())(int, int, int))(int, int, int)
{
    return func5;
}

void funcptr()
{
    assert(func2(1, 2, 3) == 6);
    int (*pfunc)() = func3(1, 2, 3);
    assert(pfunc() == 0);
    int (*pfunc2)(int, int, int) = func4(10, 20, 30);
    assert(pfunc2(4, 5, 6) == 15);
    pfunc2 = func5(0, 0, 0);
    assert(pfunc2(7, 8, 9) == 24);
    assert(func6()(0, 0, 0)(10, 11, 12) == 33);
}

int main()
{
    multi();
    change();
    arithm1();
    arithm2();
    arithm3();
    bracket();
    funcptr();

    int a = 0;
    int b = 0;
    infunc(&a, &b);
    assert(a == 30);
    assert(b == 50);

    SUCCESS;
}
