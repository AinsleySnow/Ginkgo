#include "test.h"

void logical_or()
{
    int a = 43;
    a = 0 || 384;                           assert(a == 1);
    a = 0 || a;                             assert(a == 1);
    a = a || 34 * a + 49;                   assert(a == 1);
    a = 23 || 48 * a + 38 - 16;             assert(a == 1);
    a = 34 * a + 83 || 384 * 92 + 924 - a;  assert(a == 1);
}

void logical_and()
{
    int a = 20220606;
    a = 43 && 394;                          assert(a == 1);
    a = 0 && 384;                           assert(a == 0);
    a = 0 && a;                             assert(a == 0);
    a = a && 34 * a + 49;                   assert(a == 0);
    a = 23 && 48 * a + 38 - 16;             assert(a == 1);
    a = 34 * a + 83 && 384 * 92 + 924 - a;  assert(a == 1);
}

void triop()
{
    int a = 43, b = 38, c = 29;
    int d = 38 ? 21 : 39;       assert(d == 21);
    d = a ? 21 : 39;            assert(d == 21);
    d = 38 ? a + b + c : 39;    assert(d == 110);
    d = a ? a + b + c : 39;     assert(d == 110);
    d = 38 ? 21 : a + b + c;    assert(d == 21);
    d = a ? 21 : a + b + c;     assert(d == 21);
    d = 38 ? a + b : a + b + c; assert(d == 81);
    d = a ? a + b : a + b + c;  assert(d == 81);
}

int main(void)
{
    logical_or();
    logical_and();
    triop();

    SUCCESS;
}
