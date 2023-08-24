#include "test.h"

struct verybig
{
    char a[128];
};

void bigargv(struct verybig big)
{
    assert(big.a[0] == 'a');
    assert(big.a[127] == 'b');
}

struct _8byte
{
    int a;
    int b;
    int c;
    int d;
};

void _8bytefunc(struct _8byte e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c == 40);
    assert(e.d == 50);
}

struct _8byte2
{
    int a;
    char b[8];
    int c;
};

void _8byte2func(struct _8byte2 e)
{
    assert(e.a == 20);
    assert(e.b[0] == 30);
    assert(e.b[7] == 40);
    assert(e.c == 50);
}

struct _8byte3
{
    int a;
    float b;
    int c;
    float d;
};

void _8byte3func(struct _8byte3 e)
{
    assert(e.a == 20);
    assert(e.b == 30.0);
    assert(e.c == 40);
    assert(e.d == 50.0);
}

struct _8byte4
{
    int a;
    double b;
    int c;
};

void _8byte4func(struct _8byte4 e)
{
    assert(e.a == 20);
    assert(e.b == 30.0);
    assert(e.c == 40);
}

struct _8byte5
{
    int a;
    char b[16];
    int c;
};

void _8byte5func(struct _8byte5 e)
{
    assert(e.a == 20);
    assert(e.b[0] == 30);
    assert(e.b[15] == 40);
    assert(e.c == 50);
}

struct _8byte6
{
    int a;
    struct
    {
        int b;
        int c;
    } within;
    int d;
};

void _8byte6func(struct _8byte6 e)
{
    assert(e.a == 20);
    assert(e.within.b == 30);
    assert(e.within.c == 40);
    assert(e.d == 50);
}

struct _8byte7
{
    int a;
    struct
    {
        char reallybig[128];
    } big;
    int b;
};

void _8byte7func(struct _8byte7 e)
{
    assert(e.a == 20);
    assert(e.big.reallybig[0] == 30);
    assert(e.big.reallybig[127] == 40);
    assert(e.b == 50);
}

//--- structs greater than 16 bytes but less than 64 bytes ---
struct _8byte8
{
    long a;
    long b;
    long c;
    long d;
    long e;
    long f;
//--- pass on stack ---
    long g;
    long h;
};

void _8byte8func(struct _8byte8 e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c == 40);
    assert(e.d == 50);
    assert(e.e == 60);
    assert(e.f == 70);
    assert(e.g == 80);
    assert(e.h == 90);
}

struct _8byte9
{
    long a;
    long b;
    long c;
    long d;
    long e;
    int f;
};

void _8byte9func(struct _8byte9 e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c == 40);
    assert(e.d == 50);
    assert(e.e == 60);
    assert(e.f == 70);
}

struct _8byte10
{
    long a;
    long b;
    char c[3];
};

void _8byte10func(struct _8byte10 e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c[0] == 40);
    assert(e.c[2] == 50);
}

struct _8byte11
{
    long a;
    long b;
    struct
    {
        float c;
        float d;
        double e;
        double f;
    };
};

void _8byte11func(struct _8byte11 e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c == 40.0);
    assert(e.d == 50.0);
    assert(e.e == 60.0);
    assert(e.f == 70.0);
}

struct _8byte12
{
    long a;
    long b;
    struct
    {
        float c;
        float d;
        struct
        {
            double e;
            double f;
        };
    };
};

void _8byte12func(struct _8byte12 e)
{
    assert(e.a == 20);
    assert(e.b == 30);
    assert(e.c == 40.0);
    assert(e.d == 50.0);
    assert(e.e == 60.0);
    assert(e.f == 70.0);
}

// --- 'overaligned' structs ---
struct inner
{
    long a;
    long b;
    long c;
    long d;
    long e;
    long f;
    long g;
    long h;
    char _Alignas(16) i;
};

struct _8byte13
{
    char a[48];
    struct inner b;
    long c;
};

void _8byte13func(struct _8byte13 e)
{
    assert(e.a[0] == 20);
    assert(e.a[47] == 30);
    assert(e.b.a == 40);
    assert(e.b.b == 50);
    assert(e.b.c == 60);
    assert(e.b.d == 70);
    assert(e.b.e == 80);
    assert(e.b.f == 90);
    assert(e.b.g == 100);
    assert(e.b.h == 110);
    assert(e.b.i == 120);
    assert(e.c == 130);
}

// This is adapted from the example in the abi document.
struct s
{
    int a, b;
    double d;
};

double func(int e, int f, struct s ss, int g, int h,
    double m, double n, int i, int j, int k)
{
    return e + f + ss.a + ss.b +
        ss.d + g + h + m + n + i + j + k;
}

int main()
{
    struct verybig a;
    a.a[0] = 'a';
    a.a[127] = 'b';
    bigargv(a);

    struct _8byte b;
    b.a = 20;
    b.b = 30;
    b.c = 40;
    b.d = 50;
    _8bytefunc(b);

    struct _8byte2 c;
    c.a = 20;
    c.b[0] = 30;
    c.b[7] = 40;
    c.c = 50;
    _8byte2func(c);

    struct _8byte3 d;
    d.a = 20;
    d.b = 30.0;
    d.c = 40;
    d.d = 50.0;
    _8byte3func(d);

    struct _8byte4 e;
    e.a = 20;
    e.b = 30.0;
    e.c = 40;
    _8byte4func(e);

    struct _8byte5 f;
    f.a = 20;
    f.b[0] = 30;
    f.b[15] = 40;
    f.c = 50;
    _8byte5func(f);

    struct _8byte6 g;
    g.a = 20;
    g.within.b = 30;
    g.within.c = 40;
    g.d = 50;
    _8byte6func(g);

    struct _8byte7 h;
    h.a = 20;
    h.big.reallybig[0] = 30;
    h.big.reallybig[127] = 40;
    h.b = 50;
    _8byte7func(h);

    struct _8byte8 i;
    i.a = 20;
    i.b = 30;
    i.c = 40;
    i.d = 50;
    i.e = 60;
    i.f = 70;
    i.g = 80;
    i.h = 90;
    _8byte8func(i);

    struct _8byte9 j;
    j.a = 20;
    j.b = 30;
    j.c = 40;
    j.d = 50;
    j.e = 60;
    j.f = 70;
    _8byte9func(j);

    struct _8byte10 k;
    k.a = 20;
    k.b = 30;
    k.c[0] = 40;
    k.c[2] = 50;
    _8byte10func(k);

    struct _8byte11 l;
    l.a = 20;
    l.b = 30;
    l.c = 40.0;
    l.d = 50.0;
    l.e = 60.0;
    l.f = 70.0;
    _8byte11func(l);

    struct _8byte12 m;
    m.a = 20;
    m.b = 30;
    m.c = 40.0;
    m.d = 50.0;
    m.e = 60.0;
    m.f = 70.0;
    _8byte12func(m);

    struct _8byte13 n;
    n.a[0] = 20;
    n.a[47] = 30;
    n.b.a = 40;
    n.b.b = 50;
    n.b.c = 60;
    n.b.d = 70;
    n.b.e = 80;
    n.b.f = 90;
    n.b.g = 100;
    n.b.h = 110;
    n.b.i = 120;
    n.c = 130;
    _8byte13func(n);

    struct s ss;
    ss.a = 10;
    ss.b = 11;
    ss.d = 12.0;
    double ans = func(1, 2, ss, 3, 4, 5.0, 6.0, 7, 8, 9);
    assert(ans == 78.0);
    SUCCESS;
}
