#include "tests/test.h"

void integer()
{
    int a = 1 + 1; // 2
    int b = 1 + 2 + 3; // 6
    int c = 1 - 2 + 3; // 2
    int d = 1 + 2 - 4; // -1
    int e = 1 * 2 + 3; // 5
    int f = 1 + 2 * 3; // 7
    int g = (1 + 5) * 4; // 24
    int h = (1 + 2) * (3 + 4); // 21
    int i = 1 & 2 & 3; // 0
    int ip = 45 % 8; // 5
    int j = 1 | 2 | 3; // 3
    int l = (1 | 2) | 3; // 3
    int n = -2; // -2
    int o = 3; // 3
    int p = ~4; // -5
    int q = !4; // 0
    int r = 3 << 4; // 48
    int rp = 32 >> 4; // 2
    int rpp = (1 << 31) >> 31; // -1
    int s = 3 < 4; // 1
    int t = 3 > 4; // 0
    int sp = 3 <= 4; // 1
    int tp = 3 >= 4; // 0
    int u = 3 == 4; // 0
    int v = 3 != 4; // 1
    int w = 4 & 2 ^ 3; // 3
    int x = 4 ^ 2 & 3; // 6
    int y = 4 & 2 ^ 3 | 5; // 7

    assert(a == 2);     assert(b == 6);     assert(c == 2);
    assert(d == -1);    assert(e == 5);     assert(f == 7);
    assert(g == 24);    assert(h == 21);    assert(i == 0);
    assert(ip == 5);    assert(j == 3);     assert(l == 3);
    assert(n == -2);    assert(o == 3);     assert(p == -5);
    assert(q == 0);     assert(r == 48);    assert(rp == 2);
    assert(rpp == -1);  assert(s == 1);     assert(t == 0);
    assert(sp == 1);    assert(tp == 0);    assert(u == 0);
    assert(v == 1);     assert(w == 3);     assert(x == 6);
    assert(y == 7);
}

void floatpoint()
{
    float aa = 1.23 + 4.77; // 6
    float ab = 1.23 + 4.56 + 8.21; // 14
    float ac = 1.23 - 4.56 + 7.33; // 4
    float ad = 1.23 + 4.56 - 7.79; // -2
    float ae = -23.579 * 4.56 - 200.47976; // -308
    float af = 1.0216 + 4.56 * 7.89; // 37
    float ag = (1 + 4) * 99.4; // 497
    float ah = 345 * (213.34 + 345.556) - 0.12; // 192819
    float ai = (3472.444 * 347.223) + (23.9886 * 3) - 0.388812; // 1205784

    assert(aa == 6);          assert(ab == 14);
    assert(ac == 4);          assert(ad == -2);
    assert(ae == -308);       assert(af == 37);
    assert(ag == 497);        assert(ah == 192819);
    assert(ai == 1205784);
}

void fp_comp()
{
    int aj = 4.0 == 4.0; // 1
    int ak = 4.1 == 4.0; // 0
    int al = 4.1 != 4.0; // 1
    int am = !2.3; // 0
    int an = 3.07 < 4.5657; // 1
    int ao = 3.43 > 4.98987; // 0
    int ap = 3.07 <= 4.5657; // 1
    int aq = 3.43 >= 4.98987; // 0

    float az = 1.2 == 3; // 0
    float ba = 3 == 1.2; // 0
    float bb = 3 != 1.2; // 1
    float bc = 1.2 != 3; // 1
    float bd = 3 > 1.2; // 1
    float be = 1.2 > 3; // 0
    float bf = 3 < 1.2; // 0
    float bg = 1.2 < 3; // 1
    float bh = 3 >= 1.2; // 1
    float bi = 1.2 >= 3; // 0
    float bj = 3 <= 1.2; // 0
    float bk = 1.2 <= 3; // 1

    assert(aj == 1); assert(ak == 0);
    assert(al == 1); assert(am == 0);
    assert(an == 1); assert(ao == 0);
    assert(ap == 1); assert(aq == 0);

    assert(az == 0); assert(ba == 0); assert(bb == 1);
    assert(bc == 1); assert(bd == 1); assert(be == 0);
    assert(bf == 0); assert(bg == 1); assert(bh == 1);
    assert(bi == 0); assert(bj == 0); assert(bk == 1);
}

int main()
{
    integer();
    floatpoint();
    fp_comp();

    printf("OK\n");
    return 0;
}
