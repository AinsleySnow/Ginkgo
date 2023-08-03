#include "test.h"

void common()
{
    int a[4];
    a[3] = 10;
    a[0] = 0;

    assert(a[3] == 10);
    assert(a[0] == 0);
}

void common2()
{
    int a[4];
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;

    int b = 2;
    assert(a[b] == 3);
    assert(a[b + 1] == 4);
}

void pointer()
{
    int a[4];
    int* b = a;
    *b = 3;         assert(a[0] == 3);
    *(b + 2) = 4;   assert(a[2] == 4);
    b[2] = 5;       assert(a[2] == 5);
}

void bidirectional()
{
    int a[4][5];
    a[2][3] = 10;
    assert(a[2][3] == 10);
    a[2][1] = 10;
    a[2][1] += 20;
    assert(a[2][1] == 30);
    a[1][0] = 20;
    int c = a[1][0] + 30;  
    assert(c == 50);
    a[1][2] = 4;
    c = a[1][2] * 20;
    assert(c == 80);
}

void parray()
{
    int a[3];
    int b[3];
    int c[3];
    int (*pa[3])[3];
    pa[0] = &a;
    pa[1] = &b;
    pa[2] = &c;

    (*pa[0])[1] = 20;
    assert(a[1] == 20);
    (*(pa + 1))[2] = 30;
    assert(b[2] == 30);
}

void address(int a[4])
{
    assert(a[0] == 1);
    a[0] = 2;
    int* p = &a[3];
    assert(*p == 4);
    *p = 5;
    assert(a[3] == 5);
}

int (*retparray())[3]
{
    return 0;
}

void direct()
{
    int a[4];
    *a = 4;
    assert(a[0] == 4);
    *(a + 2) = 10;
    assert(a[2] == 10);
}

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int mul(int a, int b)
{
    return a * b;
}

int div(int a, int b)
{
    return a / b;
}

void pfunc()
{
    int a = 20, b = 30;

    int (*array[4])(int, int);
    array[0] = add;
    array[1] = sub;
    array[2] = mul;
    array[3] = div;

    assert(array[0](a, b) == 50);
    assert(array[1](a, b) == -10);
    assert(array[2](a, b) == 600);
    assert(array[3](a, b) == 0);

    assert((*array)(a, b) == 50);
    assert((*(array + 1))(a, b) == -10);
    assert((*(array + 2))(a, b) == 600);
    assert((*(array + 3))(a, b) == 0);
}

void arrayargu(int a[], int b[4])
{
    assert(b[0] == 0);
    assert(b[3] == 3);
    assert(a[1] == 5);
    assert(a[9] == 20);
}

void arrayargu2(int a[4][4])
{
    assert(a[0][0] == 2023);
    assert(a[2][3] == 8);
}

void arrayargu3(int a[][4])
{
    assert(a[0][0] == 2023);
    assert(a[2][3] == 8);
}

int main()
{
    common();
    common2();
    pointer();
    bidirectional();
    parray();
    retparray();
    direct();
    pfunc();

    int a[4];
    a[0] = 1; a[1] = 2;
    a[2] = 3; a[3] = 4;
    address(a);
    assert(a[0] == 2);
    assert(a[3] == 5);

    int b[10]; b[1] = 5; b[9] = 20;
    int c[4];  c[0] = 0; c[3] = 3;
    arrayargu(b, c);

    int d[4][4];
    d[0][0] = 2023;
    d[2][3] = 8;
    arrayargu2(d);
    arrayargu3(d);

    SUCCESS;
}
