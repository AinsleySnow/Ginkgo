int global = 20;
int global2 = 30 * 379 - 47 + 15; // 11338

void t0()
{
    char a;
    unsigned char b;
    signed char c;

    short d;
    short int e;
    unsigned short f;
    signed short g;
    unsigned short int h;
    signed short int i;

    int j;
    signed k;
    unsigned l;
    signed int m;
    unsigned int n;

    long o;
    unsigned long p;
    signed long q;
    long long r;
    unsigned long long s;
    signed long long t;
    long int u;
    unsigned long int v;
    signed long int w;
    long long int x;
    unsigned long long int y;
    signed long long int z;

    float aa;
    double ab;
    long double ac;

    int ad, ae, af;
    char ag, ah, ai, aj;
    long al, am, an, ao, ap;
    short aq, ar, as, at, au, av;
    double aw, ax, ay, az;
    float ba, bb, bc, bd;
}

void t1()
{
    int a = 1976;
    int b = 7;
    int e = 28;

    int c = a + b;
    c = a + b + b;    c = a - b + e;
    c = a * b + e;    c = a / e * b;
    c = a % b + e;    c = a % e ^ b;
    c = (c * a) + (e * b - c * e);
    c = (b + a) * e;
    c = b & a & c;    c = b | a | c;
    c = (a | e) | b;  c = a << b;
    c = a >> e;       c = a == e;
    c = a != e;       c = a > e;
    c = a < e;        c = a >= e;
    c = a <= e;       c = (a < e) + b;
    c = (a > e) + e;  c = (a >= e) + e;
    c = (a <= e) + e; c = (a == e) * e;
    c = (a != e) * b; c = a & e ^ b;
    c = a ^ e & b;    c = a | b ^ e & e;

    c = a + 30;
    c = 30 + a;
    c = a + (40 * 789); // a + 31560
    c = (40 * 33) + a; // 1320 + a

    c = a - 30;
    c = 30 - a;
    c = a - (3 * 223); // a - 669
    c = (3 + 34897) - a; // 34900 - a

    c = 40 * a;
    c = a * 40;
    c = a * (2008 + 5 * 12); // a * 2068
    c = (1976 + 7 + 28) * a; // 2011 * a

    c = a / 32;
    c = 32 / a;
    c = a / (34 * 437 - 38); // a / 14820
    c = (44 * 3 - 37) / a; // 95 / a

    c = a % 49;
    c = 37 % a;
    c = a % (47 * 327 - 21); // a % 15348
    c = (37 - 2 * 3) % a; // 31 % a

    c = a & 38;
    c = 35 & a;
    c = a & (2383 * 39 - 38); // a & 92899
    c = (23 * 37 - 3 + 83) & a; // 931 & a

    c = a | 281;
    c = 197 | a;
    c = a | (27 * 37 + 28 - 271); // a | 756
    c = (28 * 27 - 28 * 26) | a; // 1 | a
    
    c = a << 24;
    c = 23 << a;
    c = a << 64; // a << 0;
    c = a << (37 * 2 + 82); // a << 156
    c = (237 * 21 + 984) << a; // 5961 << a

    c = a >> 24;
    c = 23 >> a;
    c = a >> 64; // a >> 0;
    c = a >> (37 * 2 + 82); // a >> 156
    c = (237 * 21 + 984) >> a; // 5961 >> a

    c = a > 2;
    c = 2 > a;
    c = a > (237 * 304 + 281); // a > 72329
    c = (278 * 1 + 289 - 29) > a; // 538 > a

    c = a < 2;
    c = 2 < a;
    c = a < (237 * 304 + 281); // a < 72329
    c = (278 * 1 + 289 - 29) < a; // 538 < a

    c = a >= 2;
    c = 2 >= a;
    c = a >= (237 * 304 + 281); // a >= 72329
    c = (278 * 1 + 289 - 29) >= a; // 538 >= a

    c = a <= 2;
    c = 2 <= a;
    c = a <= (237 * 304 + 281); // a <= 72329
    c = (278 * 1 + 289 - 29) <= a; // 538 <= a

    c = a == 4;
    c = 4 == a;
    c = a == (82 * 301 + 20 - 271); // a == 24431
    c = (27 * 2 - 72 + 1632) == a; // 1614 == a

    c = a != 4;
    c = 4 != a;
    c = a != (3278 * 92 - 21 / 38); // a != 301576
    c = (273 * 121 - 28 * 20) != a; // 32473 != a

    c = a & 9;
    c = 9 & a;
    c = a & (21 * 291 - 2 + 829); // a & 6938
    c = (238 * 25 - 83 + 2671) & a; // 8538 & a

    c = a | 10;
    c = 10 | a;
    c = a | (237 * 392 - 83); // a | 92821
    c = (328 * 2 - 281 + 27) | a; // 402 | a

    c = a ^ 6;
    c = 6 ^ a;
    c = (28 * 208 - 28) ^ a; // 5976 ^ a
    c = a ^ (3873 * 27 - 2813); // a ^ 101758
}

void t2()
{
    int b = 10, a = 8, c = 2;

    b = a;     b += 3;    b -= 2;
    b *= 4;    b /= 5;    b <<= 4;
    b >>= 3;   b %= 10;   b &= 28;
    b |= 11;   b ^= 483;

    b += c;    b -= c;    b *= c;
    b /= c;    b <<= c;   b >>= c;
    b %= c;    b &= c;    b |= c;
    b ^= c;

    b = a = c;    b = a += 4;
    b = a -= 3;   b = a *= 19;
    b = a /= 20;  b = a <<= 2;
    b = a >>= 14; b = a %= 2;
    b = a &= 2;   b = a |= 3;
    b = a ^= 3;
}

void t3()
{
    int a = 10;
    int b = 20;
    int z = ~b;
    z = -b;    z= !b;
    z = ~z;    z = -z;
    z = !z;

    z = ~z + b;
    z = -z + b;
    z = !z + b;

    int c = b++;    c = b++ + a;
    c = b++ + 5;    c = a + b++;
    c = 5 + b++;    c = ++b + a;
    c = ++b + 10;   c = 10 + ++b;
    c = a + ++b;

    int c = b--;    c = b-- + a;
    c = b-- + 5;    c = a + b--;
    c = 5 + b--;    c = --b + a;
    c = --b + 10;   c = 10 + --b;
    c = a + --b;
}

void t4()
{
    char a = 2;    a <<= 8;    a >>= 8;
    short b = 2;   b <<= 16;   b >>= 16;
    int c = 2;     c <<= 32;   c >>= 32;
    long d = 2;    d <<= 64;   d >>= 64;
}

void t5()
{
    double a = 1.919;   double b = 5.04;
    double c = 1.921;   double d = 7.23;
    double e = 1.949;   double f = 10.01;

    double g = a + b;   g = a - b;
    g = a * b;          g = a / b;
    g = a == b;         g = a != b;
    g = a > b;          g = a >= b;
    g = a < b;          g = a <= b;

    g = a + 2020.0123;     g = 2020.0123 + a;
    g = a - 1978.11;       g = 1978.11 - a;
    g = a * 2008.0808;     g = 2008.0808 * a;
    g = a / 2022.0204;     g = 2022.0204 / a;
    g = a == 1931.0918;    g = 1931.0918 == a;
    g = a != 2019.0205;    g = 2019.0205 != a;
    g = a > 3.1415926535;  g = 3.1415926535 > a;
    g = a < 2.718281828;   g = 2.718281828 < a;
    g = a >= 3.1415926535; g = 3.1415926535 >= a;
    g = a <= 2.718281828;  g = 2.718281828 <= a;

    g += 2020.0707;    g -= 2022.1111;
    g *= 2020.0708;    g /= 1997.0701;

    g = c += 2022.1130;    g = d -= 1939.0901;
    g = e *= 1937.0707;    g = f /= 2023.0122;
}

int main(void)
{
    t1();
    t2();
    t3();
    t4();
    t5();
    return 0;
}
