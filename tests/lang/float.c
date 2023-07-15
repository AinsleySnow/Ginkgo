#include "test.h"

void flt_var()
{
    double a = 1.919;   double b = 5.04;
    double e = 19490;   double f = 10;
    double g = 0;

    g = a + b;  assert(g + 0.041 == 7);
    g = a - b;  assert(g + 3.121 == 0);
    g = a * b;  assert(g + 0.32824 == 10);
    g = e / f;  assert(g == 1949.0);
}

void flt_cmp()
{
    double a = 1978.1222, b = 1993.1114;
    double g = 0;

    g = a == b; assert(g == 0);
    g = a != b; assert(g == 1);
    g = a > b;  assert(g == 0);
    g = a >= b; assert(g == 0);
    g = a < b;  assert(g == 1);
    g = a <= b; assert(g == 1);
}

void flt_arithm()
{
    double a = 2020.0;
    double g = 2023.0;

    g = a + 2008.0;         assert(g == 4028.0);
    g = 2020.0 + a;         assert(g == 4040.0);
    g = a - 1978.0;         assert(g == 42.0);
    g = 1978.0 - a;         assert(g == -42.0);
    g = a * 2008.0;         assert(g == 4056160.0);
    g = 808.0 * a;          assert(g == 1632160.0);
    g = a / 2020.0;         assert(g == 1.0);
    g = 2020.0 / a;         assert(g == 1.0);
    g = a == 1931.0;        assert(g == 0.0);
    g = 918.0 == a;         assert(g == 0.0);
    g = a != 2019.0;        assert(g == 1.0);
    g = 2019.0 != a;        assert(g == 1.0);
    g = a > 3.1415926535;   assert(g == 1.0);
    g = 3.1415926535 > a;   assert(g == 0.0);
    g = a < 2.718281828;    assert(g == 0.0);
    g = 2.718281828 < a;    assert(g == 1.0);
    g = a >= 3.1415926535;  assert(g == 1.0);
    g = 3.1415926535 >= a;  assert(g == 0.0);
    g = a <= 2.718281828;   assert(g == 0.0);
    g = 2.718281828 <= a;   assert(g == 1.0);

    g += 2020.0;        assert(g == 2021.0);
    g -= 707.0;         assert(g == 1314.0);
    g *= 2020.0;        assert(g == 2654280.0);
    g /= 2654280.0;     assert(g == 1.0);

    g = a += 1939.0;    assert(g == 3959.0);
    g = a -= 901.0;     assert(g == 3058.0);
    g = a *= 1937.0;    assert(g == 5923346.0);
    g = a /= 5923346.0; assert(g == 1.0);
}

int main()
{
    flt_var();
    flt_cmp();
    flt_arithm();

    SUCCESS;
}
