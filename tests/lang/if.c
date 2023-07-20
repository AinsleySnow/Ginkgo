#include "test.h"

void constant()
{
    if (1) { assert(1); }
    if (0) { assert(0); }
    if (1) { assert(1); } else { assert(0); }
    if (0) { assert(0); } else { assert(1); }

    if (1) assert(1);
    if (0) assert(0);
    if (1) assert(1); else assert(0);
    if (0) assert(0); else assert(1);
}

void expr()
{
    int a = 9;
    if (a == 8);
    if (a == 9);
    if (a == 10) { assert(0); }
    if (a == 7)  { assert(0); }
    else if (a == 2) { assert(0); }
    else if (a == 5) { assert(0); }
    else { a = 23; }

    if (a == 23) assert(1);
    if (a == 79) assert(0);
    else         assert(1);
}

int with_return(int i)
{
    if (i == 1) return 0;
    else if (i == 2) return 3;
    else if (i == 3) return 4;

    if (i > 10) return 5;
    else return 9;
}

void loop()
{
    int a = 10;
l1:
    if (a >= 0) { a -= 1; goto l1; }
    assert(a == -1);
}

void three_pass()
{
l1:
    int a = 10;
    if (a >= 0)
    {
        int b = 5;
        if (a <= 20) { goto l3; }
l2:
        if (a == 30) { goto l5; }
        else { goto l4; }
    }
l3:
    a = 30;
    goto l2;
l5:
    a = 45;
    goto l2;
l4:
    return;
}

int main()
{
    constant();
    expr();
    loop();
    three_pass();

    int r = with_return(1);
    assert(r == 0);
    r = with_return(3);
    assert(r == 4);
    r = with_return(11);
    assert(r == 5);
    r = with_return(5);
    assert(r == 9);

    SUCCESS;
}
