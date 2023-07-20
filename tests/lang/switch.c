#include "test.h"

void constant()
{
    switch (8)
    {
    case 8: assert(1); break;
    case 7: assert(0); break;
    case 6: assert(0); break;
    case 5: assert(0); break;
    }

    switch (8 + 8 * 9)
    {
    case 8: assert(0);
    case 7: assert(0);
    case 6: assert(0);
    case 5: assert(0);
    }

    switch (2)
    {
    case 3: assert(0);
    case 4: assert(0);
    }
}

void expr()
{
    int a = 5;
    switch (a)
    {
    case 1: assert(0);
    case 2: assert(0);
    case 3: assert(0);
    case 4: assert(0);
    default: break;
    }

    switch (a + 2)
    {
    case 1: assert(0); break;
    case 2: assert(0);
    case 3: assert(0);
    default: a = 20;
    case 4: assert(a == 20); a = 30; break;
    }

    assert(a == 30;)
}

int with_return(int i)
{
    switch (i)
    {
    case 20000101: return 0;
    case 19870920: return 1;
    case 198401:   return 2;
    default: break;
    }
    return 3;
}

void nest(int a)
{
    switch (a)
    {
    case 5:
        switch (a)
        {
        case 20:
            assert(0);
        default:
            break;
        }
        assert(a == 5);
        break;
    case 8:
        switch (a + 20)
        {
        case 28:
            switch (a + 30)
            {
            case 200:
                break;
            case 38:
                assert(a == 38);
                a = 40;
            case 500:
                assert(a == 40);
            default:
                break;
            }
            assert(a == 40);
            a = 500;
        }
    case 20:
        assert(a == 500);
    }
}

void with_goto()
{
    int a = 10;
    goto l1;
l3:
    switch (a)
    {
    case 20: break;
l1:
    case 21: break;
    case 30: goto l5;
l2:
    case 23: assert(a == 20);
    case 24: a = 30; goto l4;
    }
    assert(a == 10);
    a = 20;
    goto l2;

l4:
    goto l3;
l5:
    return;
}

int main()
{
    constant();
    expr();
    nest(5);
    nest(8);
    with_goto();
    assert(with_return(19870920) == 1);
    assert(with_return(19910825) == 3);

    SUCCESS;
}
