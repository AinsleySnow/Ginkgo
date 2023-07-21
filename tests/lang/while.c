#include "test.h"

void constant()
{
    while (0);
    while (0) assert(0);
}

void common()
{
    int i = 97;
    while (i < 107)
    {
        i += 1;
    }
    assert(i == 107);

    while (i < 200)
        i += 1;
    assert(i == 200);
}

void break_continue()
{
    int i = 97;
    int a = 0;
    while (i < 107)
    {
        a += i;
        while (i < 107)
        {
            if (i == 106)
            {
                a += 25;
                i += 1;
                continue;
            }
            a += i;
            i += 1;
        }
        break;
    }
    assert(a == 1031);
}

void with_goto()
{
    int i = 97, j = 20;
    while (i < 107)
        while (j < 107)
            goto l1;

l1:
    while (i < 97)
    {
l3:
        if (i > 97)
            continue;
        ; // I'll remove this when C23 grammar is fully supported.
        int j = 20;
        while (j < 200)
        {
l2:
            if (j > 100)
                j = 0;
            while (j > 100)
                goto l2;
            j += 10;
l4:
            if (j == 10)
                goto l3;
        }
        i = 100;
    }
    assert(i == 97);
}

int main()
{
    constant();
    common();
    break_continue();
    with_goto();

    SUCCESS;
}
