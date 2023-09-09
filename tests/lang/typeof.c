#include "test.h"

void common()
{
    int a = 20;
    typeof(a) b = 30;
    assert(b == 30);
    assert(sizeof(b) == 4);

    typeof(int) c = 40;
    assert(c == 40);
    assert(sizeof(c) == 4);
}

void combine()
{
    int a = 20;
    typeof(a)* b = &a;
    *b = 30;
    assert(a == 30);
    assert(sizeof(b) == 8);

    typeof(a) c[10];
    c[0] = 40;
    c[9] = 50;
    assert(c[0] == 40);
    assert(c[9] == 50);
    assert(sizeof(c) == 40);
}

typeof(1 + 1) expr()
{
    return 3;
}

void nested()
{
    int a = 20;
    typeof(typeof(a)) b = 30;
    assert(b == 30);
    assert(sizeof(b) == 4);
}

void unqual()
{
    const int a = 20;
    typeof_unqual(a) b = 30;
    b = 40;
    assert(b == 40);
    assert(sizeof(b) == 4);
}

int main()
{
    common();
    combine();
    nested();
    unqual();

    assert(expr() == 3);
    assert(sizeof(expr()) == 4);

    SUCCESS;
}
