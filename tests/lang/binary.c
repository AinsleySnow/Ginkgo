#include "test.h"

int global = 20;
int global2 = 30 * 379 - 47 + 15; // 11338

void binary1()
{
    int a = 1976;
    int b = 7;
    int e = 28;
    int c = a + b;  assert(c == 1983);
    c = a + b + b;  assert(c == 1990);
    c = a - b + e;  assert(c == 1997);
    c = a * b + e;  assert(c == 13860);
    c = a / e * b;  assert(c == 490);
    c = a % b + e;  assert(c == 30);
    c = a % e ^ b;  assert(c == 23);

    c = (c * a) + (e * b - c * e);  assert(c == 45000);
    c = (b + a) * e;    assert(c == 55524);
    c = b & a & c;      assert(c == 0);
    c = b | a | c;      assert(c == 1983);
    c = (a | e) | b;    assert(c == 1983);
    c = a << b;         assert(c == 252928);
    c = a >> e;         assert(c == 0);
}

void binary2()
{
    int a = 1976;
    int b = 7;
    int e = 28;
    int c = 0;

    c = a == e;         assert(c == 0);
    c = a != e;         assert(c == 1);
    c = a > e;          assert(c == 1);
    c = a < e;          assert(c == 0);
    c = a >= e;         assert(c == 1);
    c = a <= e;         assert(c == 0);

    c = (a < e) + b;    assert(c == 7);
    c = (a > e) + e;    assert(c == 29);
    c = (a >= e) + e;   assert(c == 29);
    c = (a <= e) + e;   assert(c == 28);
    c = (a == e) * e;   assert(c == 0);
    c = (a != e) * b;   assert(c == 7);
}

void binary3()
{
    int a = 1976;
    int b = 7;
    int e = 28;
    int c = 7;

    c = a & e ^ b;      assert(c == 31);
    c = a ^ e & b;      assert(c == 1980);
    c = a | b ^ e & e;  assert(c == 1979);

    c = a + 30;                 assert(c == 2006);
    c = 30 + a;                 assert(c == 2006);
    c = a + (40 * 789);         assert(c == 33536);
    c = (40 * 33) + a;          assert(c == 3296);
    c = a - 30;                 assert(c == 1946);
    c = 30 - a;                 assert(c == -1946);
    c = a - (3 * 223);          assert(c == 1307);
    c = (3 + 34897) - a;        assert(c == 32924);
    c = 40 * a;                 assert(c == 79040);
    c = a * 40;                 assert(c == 79040);
    c = a * (2008 + 5 * 12);    assert(c == 4086368);
    c = (1976 + 7 + 28) * a;    assert(c == 3973736);
    c = a / 32;                 assert(c == 61);
    c = 32 / a;                 assert(c == 0);
    c = a / (34 * 437 - 38);    assert(c == 0);
    c = (44 * 3 - 37) / a;      assert(c == 0);
    c = a % 49;                 assert(c == 16);
    c = 37 % a;                 assert(c == 37);
    c = a % (47 * 327 - 21);    assert(c == 1976);
    c = (37 - 2 * 3) % a;       assert(c == 31);
}

void binary4()
{
    int a = 1976;
    int c = 31;

    c = a & 38;                     assert(c == 32);
    c = 35 & a;                     assert(c == 32);
    c = a & (2383 * 39 - 38);       assert(c == 672);
    c = (23 * 37 - 3 + 83) & a;     assert(c == 928);
    c = a | 281;                    assert(c == 1977);
    c = 197 | a;                    assert(c == 2045);
    c = a | (27 * 37 + 28 - 271);   assert(c == 2044);
    c = (28 * 27 - 28 * 26) | a;    assert(c == 1980);
    c = a ^ 6;                      assert(c == 1982);
    c = 6 ^ a;                      assert(c == 1982);
    c = (28 * 208 - 28) ^ a;        assert(c == 4380);
    c = a ^ (3873 * 27 - 2813);     assert(c == 101062);
}

void binary5()
{
    int a = 1976;
    int c = 1980;

    c = a << 24;                assert(c == -1207959552);
    c = 23 << a;                assert(c == 385875968);
    c = a << 64;                assert(c == 1976);
    c = a << (37 * 2 + 82);     assert(c == -2147483648);
    c = (237 * 21 + 984) << a;  assert(c == 1224736768);
    c = a >> 24;                assert(c == 0);
    c = 23 >> a;                assert(c == 0);
    c = a >> 64;                assert(c == 1976);
    c = a >> (37 * 2 + 82);     assert(c == 0);
    c = (237 * 21 + 984) >> a;  assert(c == 0);
}

void binary6()
{
    int a = 1976;
    int c = 0;

    c = a > 2;                      assert(c == 1);
    c = 2 > a;                      assert(c == 0);
    c = a > (237 * 304 + 281);      assert(c == 0);
    c = (278 * 1 + 289 - 29) > a;   assert(c == 0);
    c = a < 2;                      assert(c == 0);
    c = 2 < a;                      assert(c == 1);
    c = a < (237 * 304 + 281);      assert(c == 1);
    c = (278 * 1 + 289 - 29) < a;   assert(c == 1);
    c = a >= 2;                     assert(c == 1);
    c = 2 >= a;                     assert(c == 0);
    c = a >= (237 * 304 + 281);     assert(c == 0);
    c = (278 * 1 + 289 - 29) >= a;  assert(c == 0);
    c = a <= 2;                     assert(c == 0);
    c = 2 <= a;                     assert(c == 1);
    c = a <= (237 * 304 + 281);     assert(c == 1);
    c = (278 * 1 + 289 - 29) <= a;  assert(c == 1);
    c = a == 4;                     assert(c == 0);
    c = 4 == a;                     assert(c == 0);
    c = a == (82 * 301 + 20 - 271); assert(c == 0);
    c = (27 * 2 - 72 + 1632) == a;  assert(c == 0);
    c = a != 4;                     assert(c == 1);
    c = 4 != a;                     assert(c == 1);
    c = a != (3278 * 92 - 21 / 38); assert(c == 1);
    c = (273 * 121 - 28 * 20) != a; assert(c == 1);
}

void arithm_assign()
{
    int b = 10, a = 8, c = 2;

    b = a;          assert(b == 8);     b += 3;         assert(b == 11);
    b -= 2;         assert(b == 9);     b *= 4;         assert(b == 36);
    b /= 5;         assert(b == 7);     b <<= 4;        assert(b == 112);
    b >>= 3;        assert(b == 14);    b %= 10;        assert(b == 4);
    b &= 28;        assert(b == 4);     b |= 11;        assert(b == 15);
    b ^= 483;       assert(b == 492);   b += c;         assert(b == 494);
    b -= c;         assert(b == 492);   b *= c;         assert(b == 984);
    b /= c;         assert(b == 492);   b <<= c;        assert(b == 1968);
    b >>= c;        assert(b == 492);   b %= c;         assert(b == 0);
    b &= c;         assert(b == 0);     b |= c;         assert(b == 2);
    b ^= c;         assert(b == 0);     b = a = c;      assert(b == 2);
    b = a += 4;     assert(b == 6);     b = a -= 3;     assert(b == 3);
    b = a *= 19;    assert(b == 57);    b = a /= 20;    assert(b == 2);
    b = a <<= 2;    assert(b == 8);     b = a >>= 14;   assert(b == 0);
    b = a %= 2;     assert(b == 0);     b = a &= 2;     assert(b == 0);
    b = a |= 3;     assert(b == 3);     b = a ^= 3;     assert(b == 0);
}

void shift_assign()
{
    char a = 2;    a <<= 8;    a >>= 8;
    short b = 2;   b <<= 16;   b >>= 16;
    int c = 2;     c <<= 32;   c >>= 32;
    long d = 2;    d <<= 64;   d >>= 64;
}

int main()
{
    assert(global == 20);
    global = global + 2;
    assert(global == 22);
    assert(global2 == 11338);

    binary1();
    binary2();
    binary3();
    binary4();
    binary5();
    binary6();
    arithm_assign();
    shift_assign();

    SUCCESS;
}
