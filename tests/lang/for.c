#include "test.h"

void constant()
{
    for (; 0; )    assert(0);
    for (1; 0; )   assert(0);
    for (0; 0; )   assert(0);
    for (int; 0; ) assert(0);

    for (int i = 97; 0; i++) assert(0);
    for (int i = 1; i; --i)  assert(1);
    for (int i = 97; 0; i++);
}

void common()
{
    int a = 0;
    for (int i = 97; i < 107; ++i)
        a += i;
    assert(a == 1015);

    a = 0;
    int i = 97;
    for (; i < 107; ++i)
        a += i;

    for (;; ++i)
    {
        if (i >= 200) break;
        a += i;
    }
    assert(a == 15244);

    for (;;)
    {
        if (i >= 300) break;
        assert(i++ < 300);
    }
    a = i;
    assert(a == 300);

    a = 0;
    for (int i = 97; i < 107; ++i)
    {
        if (i == 102) continue;
        a += i;
    }
    assert(a == 913);
}

void nest()
{
    int a = 0;
    for (int i = 1; i <= 9; ++i)
    {
        for (int j = 1; j <= 9; ++j)
        {
            a += i * j;
        }
    }
    assert(a == 2025);

    for (int i = 49; i < 58; ++i)
    {
        for (int j = 49; j <= 58; ++j)
        {
            if (j == 57) break;
            if (j == 51) continue;
            assert(j != 51);
            a += j;
        }
        if (i == 57) break;
        if (i == 51) continue;
        assert(i != 51);
        a += i;
    }
    assert(a == 5715);
}

void with_goto()
{
    int i = 0, j = 0, k = 0;
    for (; i < 10; ++i)
        for (; j < 10; ++j)
            for (; k < 10; ++k)
                goto l1;
l1:
    assert(i == 0 && j == 0 && k == 0);
    for (; i < 10; ++i)
    {
l3:
        i = 9;
        for (; j < 10; ++j)
        {
            goto l4;
l2:
            k = 20;
l4:
            if (k == 20)
                continue;
            for (; k < 10; ++k)
                goto l2;
            goto l3;
        }
    }
    assert(i == 10);
}

int main()
{
    constant();
    common();
    nest();
    with_goto();

    SUCCESS;
}
