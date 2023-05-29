#include "tests/test.h"

// implicit conversion from narrower to wider type
// a.k.a. integer promotion
void char2short()
{
    char a = 127;
    short b = 1;
    short c = a + b;
    assert(c == 128);

    a = -128;
    b = -1;
    c = a + b;
    assert(c == -129);
}

void char2int()
{
    char a = 127;
    int b = 1;
    int c = a + b;
    assert(c == 128);

    a = -128;
    b = -1;
    c = a + b;
    assert(c == -129);
}

void char2long()
{
    char a = 127;
    long b = 1;
    long c = a + b;
    assert(c == 128L);

    a = -128;
    b = -1;
    c = a + b;
    assert(c == -129L);
}

void short2int()
{
    short a = 32767;
    int b = 1;
    int c = a + b;
    assert(c == 32768);

    a = -32768;
    b = -1;
    c = a + b;
    assert(c == -32769);
}

void short2long()
{
    short a = 32767;
    long b = 1;
    long c = a + b;
    assert(c == 32768L);

    a = -32768;
    b = -1;
    c = a + b;
    assert(c == -32769L);
}

void int2long()
{
    int a = 2147483647;
    long b = 1;
    long c = a + b;
    assert(c == 2147483648L);

    a = -2147483648;
    b = -1;
    c = a + b;
    assert(c == -2147483649L);
}


// implicit conversion from integer to float-point
void char2float()
{
    char a = 1;
    float b = 1.5;
    float c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

void char2double()
{
    char a = 1;
    double b = 1.5;
    double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

/*
void char2ldouble()
{
    char a = 1;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}
*/

void short2float()
{
    short a = 1;
    float b = 1.5;
    float c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

void short2double()
{
    short a = 1;
    double b = 1.5;
    double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

/*
void short2ldouble()
{
    short a = 1;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}
*/

void int2float()
{
    int a = 1;
    float b = 1.5;
    float c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

void int2double()
{
    int a = 1;
    double b = 1.5;
    double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

/*
void int2ldouble()
{
    int a = 1;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}
*/

void long2float()
{
    long a = 1;
    float b = 1.5;
    float c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

void long2double()
{
    long a = 1;
    double b = 1.5;
    double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}

/*
void long2ldouble()
{
    long a = 1;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 2.5);

    a = -1;
    b = -1.5;
    c = a + b;
    assert(c == -2.5);
}
*/

// implicit conversion from narrower float-point to wider float-point
void float2double()
{
    float a = 1.5;
    double b = 1.5;
    double c = a + b;
    assert(c == 3.0);

    a = -1.5;
    b = -1.5;
    c = a + b;
    assert(c == -3.0);
}

/*
void float2ldouble()
{
    float a = 1.5;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 3.0);

    a = -1.5;
    b = -1.5;
    c = a + b;
    assert(c == -3.0);
}

void double2ldouble()
{
    double a = 1.5;
    long double b = 1.5;
    long double c = a + b;
    assert(c == 3.0);

    a = -1.5;
    b = -1.5;
    c = a + b;
    assert(c == -3.0);
}
*/

// implicit conversion from void* to other pointer types
void void2char()
{
    void* a = 0;
    char* b = 0;
    b = a;
    assert(b == 0);
}

void char2void()
{
    char* a = 0;
    void* b = 0;
    b = a;
    assert(b == 0);
}

int main()
{
    // including all the functions above
    char2float();
    char2double();
    //char2ldouble();
    short2float();
    short2double();
    //short2ldouble();
    int2float();
    int2double();
    //int2ldouble();
    long2float();
    long2double();
    //long2ldouble();
    float2double();
    //float2ldouble();
    //double2ldouble();
    void2char();
    char2void();

    printf("OK\n");
    return 0;
}
