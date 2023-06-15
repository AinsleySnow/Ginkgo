#include "test.h"

void common()
{
    struct s
    {
        int a;
        int b;
    };
    typedef struct s ss;

    ss v;
    v.a = 20;
    v.b = 30;
    assert(v.a == 20);
    assert(v.b == 30);
}

int func1(int a)
{
    return a * 3;
}

int func2(int a)
{
    return a + 2;
}

void common2()
{
    typedef int (*func)(int);
    func = func1;
    assert(func(2) == 6);
    func = func2;
    assert(func(3) == 5);
}

void nesting()
{
    typedef int a;
    typedef a b;
    typedef b c;
    c integer = 20;
    assert(integer == 20);
}

void scope()
{
    typedef int a;

    {
        int a = 20;
        assert(a == 20);
    }
    {
        a a = 30;
        assert(a == 30);
    }

    enum A : a
    {
        A_A,
        A_B,
        A_C
    };
    assert(A_A == 0 && A_B == 1 && A_C == 2);
}

typedef int a;
int typefunc(a a)
{
    return a * 4;
}

int main()
{
    common();
    common2();
    nesting();
    scope();
    assert(typefunc(2) == 8);
}
