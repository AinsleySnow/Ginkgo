#include "test.h"
void exit(int);

int add(int, int), sub(int x, int);
double mul(double a, double b);

int many_int_argv(
    int, int, int, int, int, int, int,
    int, int, int, int, int, int, int);
float many_float_argv(
    float, float, float, float, float, float, float,
    float, float, float, float, float, float, float);
float mix1(
    int, int, int, int, int, int,
    float, float, float, float, float, float);
float mix2(
    int, float, int, float, int, float, int, float,
    int, float, int, float, int, float, int, float);
float mix3(
    int, int, int, int, int, int,
    float, float, float, float, float, float,
    int, int, int, int, int, int,
    float, float, float, float, float, float);

void _void(void);
void _void2();
int _void3();

inline int add_up(int a, int b, int c)
{
    return a + b + c;
}

_Noreturn void no_return(int a)
{
    exit(a);
}

int main(void)
{
    int putchar(int);
    putchar(97);

    int sum = add(2, 3);
    assert(sum == 5);

    double product = mul(4, 9);
    assert(product == 36.0);

    int diff = sub(4, 202);
    assert(diff == -4);

    int sum2 = many_int_argv(
        0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13);
    assert(sum2 == 91);

    float sum3 = many_float_argv(
        0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
        7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0);
    assert(sum3 == 91.0);

    float sum4 = mix1(
        1, 2, 3, 4, 5, 6,
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    assert(sum4 == 42.0);
    float sum5 = mix2(
        1, 2.0, 3, 4.0, 5, 6.0, 7, 8.0,
        9, 10.0, 11, 12.0, 13, 14.0, 15, 16.0);
    assert(sum5 == 136.0);
    float sum6 = mix3(
        1, 2, 3, 4, 5, 6,
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
        7, 8, 9, 10, 11, 12,
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    assert(sum6 == 120.0);

    _void();

    printf("\nOK\n");
    no_return(0);
}

int add(int a, int b)
{
    return a + b;
}

double mul(double a, double b)
{
    return a * b;
}

int sub(int a, int)
{
    return a - 8;
}

int many_int_argv(
    int a, int b, int c, int d, int e, int f, int g,
    int h, int i, int j, int k, int l, int m, int n)
{
    return a + b + c + d + e + f + g +
        h + i + j + k + l + m + n;
}

float many_float_argv(
    float a, float b, float c, float d, float e, float f, float g,
    float h, float i, float j, float k, float l, float m, float n)
{
    return a + b + c + d + e + f + g +
        h + i + j + k + l + m + n;
}

float mix1(
    int num1, int num2, int num3, int num4, int num5, int num6,
    float num7, float num8, float num9, float num10, float num11, float num12)
{
    
    float sum = num1 + num2 + num3 + num4 + num5 + num6 +
                num7 + num8 + num9 + num10 + num11 + num12;
    return sum;
}

float mix2(
    int num1, float num2, int num3, float num4,
    int num5, float num6, int num7, float num8,
    int num9, float num10, int num11, float num12,
    int num13, float num14, int num15, float num16)
{

    float sum = num1 + num2 + num3 + num4 +
        num5 + num6 + num7 + num8 +
        num9 + num10 + num11 + num12 +
        num13 + num14 + num15 + num16;
    return sum;
}

float mix3(
    int a, int b, int c, int d, int e, int f,
    float g, float h, float i, float j, float k, float l,
    int m, int n, int o, int p, int q, int r,
    float s, float t, float u, float v, float w, float x) {

    float sum = a + b + c + d + e + f + g + h +
        i + j + k + l + m + n + o + p +
        q + r + s + t + u + v + w + x;
    return sum;
}

void _void(void)
{
}

void _void2(void)
{
    ;;;
}

int _void3()
{
    return 0;
}
