#include "test.h"

void common()
{
    auto a = 1;
    assert(sizeof(a) == 4);

    auto b = 2.5;
    assert(sizeof(b) == 8);

    auto pa = &a;
    assert(*pa == 1);
}

void unqual()
{
    const int a = 20;
    auto b = a;
    b *= 2;
    assert(b == 40);
}

void scope()
{
    // I would like to treat auto a = a * a as a vaild
    // expression, with the a to the right of the equal sign
    // referring to the a declared in the outer scope.
    // This is invalid in C++ and violates the meaning of
    // example 2 in 6.7.9[5] of n3096.
    // Things like auto a = &a with the a not being declared
    // in the outer scope, is still invalid, though.
    double a = 7;
    double b = 9;
    {
        double b = b * b;
        assert(b == 81);
        assert(a == 7);
        auto a = a * a;
        assert(a == 49);
    }
    {
        auto b = a * a;
        assert(b == 49);
        auto a = b;
        assert(a == 49);
    }
}

void array()
{
    double array[3];
    auto pa = array;
    // pa is double*.
    assert(sizeof(pa == 8));

    auto qa = &array;
    // qa is double (*)[3].
    assert(sizeof(qa == 8));
}

int func1(int a)
{
    return a + 3;
}

int func2(int a)
{
    return a * 2;
}

void func()
{
    auto pfunc = func1;
    assert(pfunc(2) == 5);
    pfunc = func2;
    assert(pfunc(3) == 6);
}


// For such a declaration that is the definition of an object
// the init-declarator shall have the form
//     direct-declarator = assignment-expression (6.7.9[1])
// That is, return type inference (i.e. auto func(int) { return 3; })
// is not necessary in C23. Perhaps I will implement the feature someday.

int main()
{
    common();
    unqual();
    scope();
    array();
    func();

    SUCCESS;
}
