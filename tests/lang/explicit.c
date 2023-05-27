#include <assert.h>

// forced promotion of signed integer types
void char2short()
{
    char a = 127;
    short b = 0;
    b = (short) a;
    assert(b == 127);

    a = -1;
    b = (short) a;
    assert(b == -1);
}

void char2int()
{
    char a = 127;
    int b = 0;
    b = (int) a;
    assert(b == 127);

    a = -1;
    b = (int) a;
    assert(b == -1);
}

void short2int()
{
    short a = 32767;
    int b = 0;
    b = (int) a;
    assert(b == 32767);

    a = -1;
    b = (int) a;
    assert(b == -1);
}

void char2long()
{
    char a = 127;
    long b = 0;
    b = (long) a;
    assert(b == 127);

    a = -1;
    b = (long) a;
    assert(b == -1);
}

void int2long()
{
    int a = 2147483647;
    long b = 0;
    b = (long) a;
    assert(b == 2147483647);

    a = -1;
    b = (long) a;
    assert(b == -1);
}

void short2long()
{
    short a = 32767;
    long b = 0;
    b = (long) a;
    assert(b == 32767);

    a = -1;
    b = (long) a;
    assert(b == -1);
}

// forced demotion of signed integer types
void short2char()
{
    short a = -1;
    char b = 0;
    b = (char) a;
    assert(b == -1);

    a = 128;
    b = (char) a;
    assert(b == -128);

    a = 384;
    b = (char) a;
    assert(b == -128);
}

void int2char()
{
    int a = -1;
    char b = 0;
    b = (char) a;
    assert(b == -1);

    a = 128;
    b = (char) a;
    assert(b == -128);

    a = 384;
    b = (char) a;
    assert(b == -128);
}

void int2short()
{
    int a = -1;
    short b = 0;
    b = (int) a;
    assert(b == -1);

    a = 32768;
    b = (short) a;
    assert(b == -32768);

    a = 98304;
    b = (short) a;
    assert(b == -32768);
}

void long2char()
{
    long a = -1;
    char b = 0;
    b = (char) a;
    assert(b == -1);

    a = 128;
    b = (char) a;
    assert(b == -128);

    a = 384;
    b = (char) a;
    assert(b == -128);
}

void long2short()
{
    long a = -1;
    short b = 0;
    b = (short) a;
    assert(b == -1);

    a = 32768;
    b = (short) a;
    assert(b == -32768);

    a = 98304;
    b = (short) a;
    assert(b == -32768);
}

void long2int()
{
    long a = -1;
    int b = 0;
    b = (int) a;
    assert(b == -1);

    a = 2147483648;
    b = (int) a;
    assert(b == -2147483648);

    a = 6442450944;
    b = (int) a;
    assert(b == -2147483648);
}

// forced conversion of equal-ranked types
void char2uchar()
{
    char a = 127;
    unsigned char b = 0;
    b = (unsigned char) a;
    assert(b == 127);

    a = -1;
    b = (unsigned char) a;
    assert(b == 255);
}

void short2ushort()
{
    short a = 32767;
    unsigned short b = 0;
    b = (unsigned short) a;
    assert(b == 32767);

    a = -1;
    b = (unsigned short) a;
    assert(b == 65535);
}

void int2uint()
{
    int a = 2147483647;
    unsigned int b = 0;
    b = (unsigned int) a;
    assert(b == 2147483647);

    a = -1;
    b = (unsigned int) a;
    assert(b == 4294967295);
}

void long2ulong()
{
    long a = 9223372036854775807;
    unsigned long b = 0;
    b = (unsigned long) a;
    assert(b == 9223372036854775807);

    a = -1;
    b = (unsigned long) a;
    assert(b == 9223372036854775807);
}

// forced promotion of floating-point types
void float2double()
{
    float a = 1.0;
    double b = 0.0;
    b = (double) a;
    assert(b == 1.0);
}

/*
commented, since I mistook the size of long double as 8 bytes.
It's... sort of feature of Ginkgo? Anyway, I'll fix it later.

void float2ldouble()
{
    float a = 1.0;
    long double b = 0.0;
    b = (long double) a;
    assert(b == 1.0);
}

void double2ldouble()
{
    double a = 1.0;
    long double b = 0.0;
    b = (long double) a;
    assert(b == 1.0);
}
*/

// forced demotion of floating-point types
void double2float()
{
    double a = 1.0;
    float b = 0.0;
    b = (float) a;
    assert(b == 1.0);
}

/*
void ldouble2float()
{
    long double a = 1.0;
    float b = 0.0;
    b = (float) a;
    assert(b == 1.0);
}

void ldouble2double()
{
    long double a = 1.0;
    double b = 0.0;
    b = (double) a;
    assert(b == 1.0);
}
*/

// conversion from floating-point to integer
void float2char()
{
    float a = 127.0;
    char b = 0;
    b = (char) a;
    assert(b == 127);

    a = -1.0;
    b = (char) a;
    assert(b == -1);

    a = 128.0;
    b = (char) a;
    assert(b == -128);

    a = 384.0;
    b = (char) a;
    assert(b == -128);
}

void double2char()
{
    double a = 127.0;
    char b = 0;
    b = (char) a;
    assert(b == 127);

    a = -1.0;
    b = (char) a;
    assert(b == -1);

    a = 128.0;
    b = (char) a;
    assert(b == -128);

    a = 384.0;
    b = (char) a;
    assert(b == -128);
}

/*
void ldouble2char()
{
    long double a = 127.0;
    char b = 0;
    b = (char) a;
    assert(b == 127);

    a = -1.0;
    b = (char) a;
    assert(b == -1);

    a = 128.0;
    b = (char) a;
    assert(b == -128);

    a = 384.0;
    b = (char) a;
    assert(b == -128);
}
*/

void float2short()
{
    float a = 32767.0;
    short b = 0;
    b = (short) a;
    assert(b == 32767);

    a = -1.0;
    b = (short) a;
    assert(b == -1);

    a = 32768.0;
    b = (short) a;
    assert(b == -32768);

    a = 98304.0;
    b = (short) a;
    assert(b == -32768);
}

void double2short()
{
    double a = 32767.0;
    short b = 0;
    b = (short) a;
    assert(b == 32767);

    a = -1.0;
    b = (short) a;
    assert(b == -1);

    a = 32768.0;
    b = (short) a;
    assert(b == -32768);

    a = 98304.0;
    b = (short) a;
    assert(b == -32768);
}

/*
void ldouble2short()
{
    long double a = 32767.0;
    short b = 0;
    b = (short) a;
    assert(b == 32767);

    a = -1.0;
    b = (short) a;
    assert(b == -1);

    a = 32768.0;
    b = (short) a;
    assert(b == -32768);

    a = 98304.0;
    b = (short) a;
    assert(b == -32768);
}
*/

void float2int()
{
    float a = 2147483647.0;
    int b = 0;
    b = (int) a;
    assert(b == 2147483647);

    a = -1.0;
    b = (int) a;
    assert(b == -1);

    a = 2147483648.0;
    b = (int) a;
    assert(b == -2147483648);

    a = 6442450944.0;
    b = (int) a;
    assert(b == -2147483648);
}

void double2int()
{
    double a = 2147483647.0;
    int b = 0;
    b = (int) a;
    assert(b == 2147483647);

    a = -1.0;
    b = (int) a;
    assert(b == -1);

    a = 2147483648.0;
    b = (int) a;
    assert(b == -2147483648);

    a = 6442450944.0;
    b = (int) a;
    assert(b == -2147483648);
}

/*
void ldouble2int()
{
    long double a = 2147483647.0L;
    int b = 0;
    b = (int) a;
    assert(b == 2147483647);

    a = -1.0L;
    b = (int) a;
    assert(b == -1);

    a = 2147483648.0L;
    b = (int) a;
    assert(b == -2147483648);

    a = 6442450944.0L;
    b = (int) a;
    assert(b == 0);
}
*/

void float2long()
{
    float a = 9223372036854775807.0f;
    long b = 0;
    b = (long) a;
    assert(b == 9223372036854775807L);

    a = -1.0f;
    b = (long) a;
    assert(b == -1L);

    a = 9223372036854775808.0f;
    b = (long) a;
    assert(b == -9223372036854775808L);
}

void double2long()
{
    double a = 9223372036854775807.0;
    long b = 0;
    b = (long) a;
    assert(b == 9223372036854775807L);

    a = -1.0;
    b = (long) a;
    assert(b == -1L);

    a = 9223372036854775808.0;
    b = (long) a;
    assert(b == -9223372036854775808L);
}

void ldouble2long()
{
    long double a = 9223372036854775807.0;
    long b = 0;
    b = (long) a;
    assert(b == 9223372036854775807L);

    a = -1.0L;
    b = (long) a;
    assert(b == -1L);

    a = 9223372036854775808.0L;
    b = (long) a;
    assert(b == -9223372036854775808L);
}

// integer to float-point
void integer2float()
{
    char a = 1;
    short b = 2;
    int c = 3;
    long d = 4;
    float e = 0;

    e = (float) a;
    assert(e == 1.0f);
    e = (float) b;
    assert(e == 2.0f);
    e = (float) c;
    assert(e == 3.0f);
    e = (float) d;
    assert(e == 4.0f);
}

void integer2double()
{
    char a = 1;
    short b = 2;
    int c = 3;
    long d = 4;
    double e = 0;

    e = (double) a;
    assert(e == 1.0);
    e = (double) b;
    assert(e == 2.0);
    e = (double) c;
    assert(e == 3.0);
    e = (double) d;
    assert(e == 4.0);
}

/*
void integer2ldouble()
{
    char a = 1;
    short b = 2;
    int c = 3;
    long d = 4;
    long double e = 0;

    e = (long double) a;
    assert(e == 1.0L);
    e = (long double) b;
    assert(e == 2.0L);
    e = (long double) c;
    assert(e == 3.0L);
    e = (long double) d;
    assert(e == 4.0L);
}
*/

// casting between different types of pointers
void pointer2pointer()
{
    int a = 0;
    int* b = &a;
    char* c = (char*) b;
    assert(c == (char*) &a);
}

void void2pointer()
{
    int a = 0;
    void* b = &a;
    assert(a == *((int*)b));
}

// convert a pointer to an integer
void pointer2integer()
{
    int a = 0;
    int* b = &a;
    int c = (int) b;
    assert(c == (int) &a);
}

// convert an integer to a pointer
void integer2pointer()
{
    int a = 0;
    int* b = (int*) a;
    assert(b == (int*) 0);
}
