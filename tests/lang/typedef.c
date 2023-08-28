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
    func f = func1;
    assert(f(2) == 6);
    f = func2;
    assert(f(3) == 5);
}

void nesting()
{
    typedef int a;
    typedef a b;
    typedef b c;
    c integer = 20;
    assert(integer == 20);
}

void array()
{
    typedef int a[20];
    a b;
    b[0] = 20;
    b[19] = 30;
    assert(b[0] == 20);
    assert(b[19] == 30);
}

// Note: It would be hard to correctly parse the code below,
// because in a bison-generated LALR parser it is difficult to
// identify whether a 'word' is just an identifier or a typedef
// name based on the context when both options seem acceptable,
// while using a GLR parser is somehow too costly.
// Perhaps I will replace the bison front end with a recursive descent
// parser, making it easier to parse highly context-related code.

/*void scope()
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

    /*enum A : a { A_A, A_B, A_C };
    assert(A_A == 0 && A_B == 1 && A_C == 2);

    {
        enum B { a, b, c };
        assert(a == 0);
    }

    {
        enum C : a { a, b, c };
        assert(a == 0);
    }

    {
        struct a { int a; int b; int c; };
        struct a aa;
        aa.a = 0;
        assert(aa.a == 0);
    }
}

typedef int a;
int typefunc(a a)
{
    return a * 4;
}

typedef int b, c;
int typefunc2(a a, b b, c c)
{
    return a + b + c;
}

int typefunc3(int (*a)(a, b, c))
{
    return a(1, 2, 3);
}

int typefunc4(int (*a)(a a, b b, c c))
{
    return a(1, 2, 3);
}

int typefunc5(int (*a)(a, int b, c))
{
    return a(1, 2, 3);
}

int typefunc6(int (*a)(int (*b)(a, b, c), a a, int))
{
    return a(typefunc2, 1, 2);
}*/

int main()
{
    common();
    common2();
    nesting();
    array();
    // scope();
    // assert(typefunc(2) == 8);

    SUCCESS;
}
