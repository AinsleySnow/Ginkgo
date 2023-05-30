#include "tests/test.h"

struct s
{
    int a;
    int b;
    int c;
};

void dot_assign()
{
    struct s ss;
    ss.a = 5;
    ss.b = 6;
    ss.c = 7;

    assert(ss.a == 5);
    assert(ss.b == 6);
    assert(ss.c == 7);
}

void dot_expr()
{
    struct s ss;
    ss.a = 5;
    int d = ss.a * 5;

    assert(ss.a == 5);
    assert(d == 25);
}

void arrow_assign()
{
    struct s ss;
    struct s* p = &ss;
    p->a = 5;
    p->b = 6;
    p->c = 7;

    assert(ss.a == 5);
    assert(ss.b == 6);
    assert(ss.c == 7);
}

void arrow_expr()
{
    struct s ss;
    struct s* p = &ss;
    p->a = 5;
    int d = p->a * 5;

    assert(ss.a == 5);
    assert(d == 25);
}

struct t
{
    char a;
    short b;
    int c;
    long d;
};

struct u
{
    long a;
    int b;
    short c;
    char d;
};

void padding()
{
    struct t vt;
    long pa = (long)&vt.a;
    long pb = (long)&vt.b;
    long pc = (long)&vt.c;
    long pd = (long)&vt.d;

    assert(sizeof(struct t) == 16);
    assert(pb - pa == 2);
    assert(pc - pb == 4);
    assert(pd - pc == 8);

    struct u vu;
    pa = (long)&vu.a;
    pb = (long)&vu.b;
    pc = (long)&vu.c;
    pd = (long)&vu.d;

    assert(sizeof(struct u) == 16);
    assert(pb - pa == 8);
    assert(pc - pb == 4);
    assert(pd - pc == 2);
}

void size()
{
    struct w
    {
        int i;
        int j;
        char c;
    } vw;

    assert(sizeof(vw) == 12);
}

void cast()
{
    struct s1
    {
        struct s2 { int a, int b } f1;
        struct s3 { int c, int d } f2;
    } s;

    s.f1.a = 10;
    s.f1.b = 20;
    s.f2.c = 30;
    s.f2.d = 40;

    assert(s.f1.a == 10);
    assert(s.f1.b == 20);
    assert(s.f2.c == 30);
    assert(s.f2.d == 40);
}

void unnamed()
{
    struct
    {
        int i;
        int j;
        char c;
    } vw;

    vw.i = 5;
    vw.j = 6;
    vw.c = 7;

    assert(vw.i == 5);
    assert(vw.j == 6);
    assert(vw.c == 7);
}

void anonymous()
{
    struct
    {
        struct { int i; int j; };
        struct { long k; long l; } w;
        int m;
    } vw;

    vw.i = 5;
    vw.j = 6;
    vw.w.k = 7;
    vw.w.l = 8;

    assert(vw.i == 5);
    assert(vw.j == 6);
    assert(vw.w.k == 7);
    assert(vw.w.l == 8);
}

int main()
{
    dot_assign();
    dot_expr();
    arrow_assign();
    arrow_expr();
    padding();
    size();
    unnamed();
    anonymous();

    printf("OK\n");
    return 0;
}
