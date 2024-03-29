#include "test.h"

union u
{
    char a;
    short b;
    int c;
};

void dot_assign()
{
    union u uu;
    uu.a = 5;
    uu.b = 6;
    uu.c = 7;

    assert(uu.a == 7);
    assert(uu.b == 7);
    assert(uu.c == 7);
}

void dot_expr()
{
    union u uu;
    uu.a = 5;
    int d = uu.a * 5;

    assert(uu.a == 5);
    assert(d == 25);
}

void arrow_assign()
{
    union u uu;
    union u* p = &uu;
    p->a = 5;
    p->b = 6;
    p->c = 7;

    assert(uu.a == 7);
    assert(uu.b == 7);
    assert(uu.c == 7);
}

void arrow_expr()
{
    union u uu;
    union u* p = &uu;
    p->a = 5;
    int d = p->a * 5;

    assert(uu.a == 5);
    assert(d == 25);
}

void size()
{
    union v
    {
        long a;
        int b;
        char c;
    };

    assert(sizeof(union u) == 4);
    assert(sizeof(union v) == 8);
}

void cast()
{
    struct v
    {
        union
        {
            struct { int i, j; };
            struct { long k, l; } w;
        };
        int m;
    } v1;

    v1.i = 2;
    v1.w.k = 5;

    assert(sizeof(struct v) == 24);
    assert(v1.i == 5);
}

int main()
{
    dot_assign();
    dot_expr();
    arrow_assign();
    arrow_expr();
    size();
    cast();

    SUCCESS;
}
