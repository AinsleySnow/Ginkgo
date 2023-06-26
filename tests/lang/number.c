#include "test.h"

int a = 1;
int b = 01;
int c = 010; // 8
int d = 0x1;
int e = 0x10;     // 16
int f = 0xABCDEF; // 11259375
int g = 0xabcdef; // 11259375
int h = 0b1;
int i = 0b10; // 2

int j = 12'34'56;
int k = 012'34'56;  // 42798
int l = 0x12'34'56; // 1193046
int m = 0b10'10'10; // 42

int n = 0l;
int o = 0L;
int p = 0ul;
int q = 0UL;
int r = 0ll;
int s = 0LL;
int t = 0ull;
int u = 0ULL;

float v = 1.0f;
float w = 1.0F;
double x = 1.0l;
double y = 1.0L;

float z = 0x1.0p1;
float aa = 0xABCDEF.ABCDEFP+6;
float ab = 1.0e-1;
float ac = 1.0e+1;

int main()
{
    assert(a == 1);
    assert(b == 01);
    assert(c == 010); // 8
    assert(d == 0x1);
    assert(e == 0x10);     // 16
    assert(f == 0xABCDEF); // 11259375
    assert(g == 0xabcdef); // 11259375
    assert(h == 0b1);
    assert(i == 0b10); // 2

    assert(j == 12'34'56);
    assert(k == 012'34'56);  // 42798
    assert(l == 0x12'34'56); // 1193046
    assert(m == 0b10'10'10); // 42

    assert(n == 0l);
    assert(o == 0L);
    assert(p == 0ul);
    assert(q == 0UL);
    assert(r == 0ll);
    assert(s == 0LL);
    assert(t == 0ull);
    assert(u == 0ULL);

    assert(v == 1.0f);
    assert(w == 1.0F);
    assert(x == 1.0l);
    assert(y == 1.0L);

    assert(z == 0x1.0p1);
    assert(aa == 0xABCDEF.ABCDEFP+6);
    assert(ab == 1.0e-1);
    assert(ac == 1.0e+1);

    return 0;
}
