int putchar(int c);

// see whether SimpleAlloc can allocate
// different-size variables in the red zone
void t1()
{
    int a = 1;
    int b = 2;
    char c = 'a';
    long d = 20;
    long e = 30;
}

// what if the size need to be
// allocated exceeds the red zone?
void t2()
{
    char c[128];
    int a = 20;
    int b = 30;
    c[0] = 40;
    c[127] = 50;
}

// what if there's function calling?
void t3()
{
    int a = 1;
    int b = 2;
    char c = 'a';
    long d = 20;
    long e = 30;
    putchar(c);
}

// basic arithmetic
void t4()
{
    int a = 1;
    int b = 2;
    int c = 3;
    c = a + b;
    c = a - b;
    c = a * b;
}

// basic comparison
void t5()
{
    int a = 1;
    int b = 2;
    int c = 3;
    c = a == b;
    c = a != b;
    c = a < b;
    c = a > b;
    c = a <= b;
    c = a >= b;
}

// basic logic
void t6()
{
    int a = 1;
    int b = 2;
    int c = 3;
    c = a && b;
    c = a || b;
    c = !a;
}

// division and modulo
void t7()
{
    int a = 1;
    int b = 2;
    int c = 3;
    c = a / b;
    c = a % b;
}

// bit operations
void t8()
{
    int a = 1;
    int b = 2;
    int c = 3;
    c = a & b;
    c = a | b;
    c = a ^ b;
    c = ~a;
    c = a << b;
    c = a >> b;
}

// signed integer conversion
void t9()
{
    int a = 1;
    short b = 2;
    long c = 3;
    char d = 4;

    int aa = d;
    aa = b;
    aa = (int)c;
}

// unsigned integer conversion
void t10()
{
    unsigned int a = 1;
    unsigned short b = 2;
    unsigned long c = 3;
    unsigned char d = 4;

    unsigned int aa = d;
    aa = b;
    aa = (unsigned int)c;
}

// can SimpleAlloc allocate float points correctly?
void t11()
{
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;
    c = a + b;
    c = a - b;
    c = a * b;
    c = a / b;

    double d = 1.0;
    double e = 2.0;
    double f = 3.0;
    f = d + e;
    f = d - e;
    f = d * e;
}

// float points comparsion
void t12()
{
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;
    c = a == b;
    c = a != b;
    c = a < b;
    c = a > b;
    c = a <= b;
    c = a >= b;

    double d = 1.0;
    double e = 2.0;
    double f = 3.0;
    f = d == e;
    f = d != e;
    f = d < e;
    f = d > e;
    f = d <= e;
    f = d >= e;
}

// float points logic
void t13()
{
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;
    c = a && b;
    c = a || b;
    c = !a;

    double d = 1.0;
    double e = 2.0;
    double f = 3.0;
    f = d && e;
    f = d || e;
    f = !d;
}

// float points conversion
void t14()
{
    float a = 1.0;
    double b = 2.0;
    float c = 3.0;
    double d = 4.0;

    float aa = b;
    aa = (float)d;
    double bb = a;
    bb = (double)c;
}

// float points to integers
void t15()
{
    float a = 1.0;
    double b = 2.0;
    float c = 3.0;
    double d = 4.0;

    int aa = (int)a;
    aa = (double)b;

    unsigned int bb = (unsigned int)a;
    bb = (unsigned int)b;
    bb = (unsigned int)c;
    bb = (unsigned int)d;
}

// integers to float points
void t16()
{
    int a = 1;
    unsigned int b = 2;
    short c = 3;
    unsigned short d = 4;
    long e = 5;
    unsigned long f = 6;
    char g = 7;
    unsigned char h = 8;

    float aa = (float)a;
    aa = (float)b;
    aa = (float)c;
    aa = (float)d;
    aa = (float)e;
    aa = (float)f;
    aa = (float)g;
    aa = (float)h;

    double bb = (double)a;
    bb = (double)b;
    bb = (double)c;
    bb = (double)d;
    bb = (double)e;
    bb = (double)f;
    bb = (double)g;
    bb = (double)h;
}

// before we run mem2reg pass, these are the only two
// cases where the frontend may generate a phi instruction
// and this function also tests whether SimpleAlloc
// can tackle SelectInst correctly
void t17(int a)
{
    int d = a ? 1 : 2;
    int e = a || 3;
    int f = a && 4;
}

// test ptrtoi and itoptr
void t18()
{
    int a = 1;
    int* b = &a;
    unsigned long g = (unsigned long)b;
    int* h = (int*)g;
}

// test bitcast
void t19()
{
    int a = 1;
    void* b = (void*)&a;
    int* c = (int*)b;
    void* d = (void*)c;
}

// test br instrcution
void t20()
{
    int a = 1;
    int b = 2;
    if (a == b)
        a = 3;
    else
        a = 4;
}

// test ret instruction
int t21(int a)
{
    return a;
}
