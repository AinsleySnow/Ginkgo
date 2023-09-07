// Copyright 2016 wgtdkp. Released under the MIT license.

#include "test.h"
#include <stdarg.h>

int putchar(int);

int sumi(int a, ...)
{
    va_list args;
    va_start(args, a);
    int acc = 0;
    for (int i = 0; i < a; ++i)
        acc += va_arg(args, int);
    va_end(args);
    return acc;
}

int maxi(int n, ...)
{
    va_list args;
    va_start(args, n);
    int max = 0x80000000;
    for (int i = 0; i < n; ++i)
    {
        int x = va_arg(args, int);
        if (x > max)
            max = x;
    }
    va_end(args);
    return max;
}

float maxf(int n, ...)
{
    va_list args;
    va_start(args, n);
    float max = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        float x = va_arg(args, float);
        if (x > max)
            max = x;
    }
    va_end(args);
    return max;
}

float sumf(int a, ...)
{
    va_list args;
    va_start(args, a);
    float acc = 0;
    for (int i = 0; i < a; ++i)
        acc += va_arg(args, float);
    va_end(args);
    return acc;
}

double sumif(int n, ...)
{
    va_list args;
    va_start(args, n);
    double acc = 0;
    for (int i = 0; i + 1 < n; i += 2)
    {
        acc += va_arg(args, int);
        acc += va_arg(args, float);
    }
    va_end(args);
    return acc;
}

typedef struct
{
    int x;
    int y;
} pos_t;

int test_struct(int a1, int a2, int a3, int a4, int a5, int a6, int a7, ...)
{
    va_list args;
    va_start(args, a7);
    pos_t pos = va_arg(args, pos_t);
    return pos.x + pos.y;
}

// new in C23
int unknown6(...)
{
    int sum = 0;
    va_list args;
    // new in C23: variadic va_start
    // In fact you can write things like "va_start(args, 1, 2, 3)"
    // but that's meaningless and CodeChk (will) report it as an error.
    va_start(args);

    for (int i = 0; i < 6; ++i)
    {
        int num = va_arg(args, int);
        sum += num;
    }
    return sum;
}

int unknown12(...)
{
    int sum = 0;
    va_list args;
    va_start(args);

    for (int i = 0; i < 12; ++i)
    {
        int num = va_arg(args, int);
        sum += num;
    }
    va_end(args);
    return sum;
}

void test()
{
    assert(sumi(8, 0, 1, 2, 3, 4, 5, 6, 7) == 28);
    assert(sumi(21, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20) == 210);
    assert(maxi(2, 1, 2) == 2);
    assert(maxi(9, -4, 6, 8, 3, 9, 11, 32, 44, 29) == 44);
    assert(maxf(9, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f) == 9.0f);
    assert(sumf(9, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f) == 45.0f);
    assert(sumif(20, 1, 2.0f, 3, 4.0f, 5, 6.0f, 7, 8.0f, 9, 10.0f, 11, 12.0f, 13, 14.0f, 15, 16.0f, 17, 18.0f, 19, 20.0f) == 210.0f);

    pos_t pos;
    pos.x = 3;
    pos.y = 7;
    assert(test_struct(1, 2, 3, 4, 5, 6, 7, pos) == 10);

    assert(unknown6(1, 2, 3, 4, 5, 6) == 21);
    assert(unknown12(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) == 78);
}

int main()
{
    test();
    SUCCESS;
}
