int putchar(int);

void t1()
{
    int a = 43;
    a = 0 || 384;
    a = 0 || a;
    a = a || 34 * a + 49;
    a = 23 || 48 * a + 38 - 16;
    a = 34 * a + 83 || 384 * 92 + 924 - a;

    a = 43 && 394;
    a = 0 && 384;
    a = 0 && a;
    a = a && 34 * a + 49;
    a = 23 && 48 * a + 38 - 16;
    a = 34 * a + 83 && 384 * 92 + 924 - a;
}


void t2()
{
    int a = 43, b = 38, c = 29;
    int d = 38 ? 21 : 39;
    d = a ? 21 : 39;
    d = 38 ? a + b + c : 39;
    d = a ? a + b + c : 39;
    d = 38 ? 21 : a + b + c;
    d = a ? 21 : a + b + c;
    d = 38 ? a + b : a + b + c;
    d = a ? a + b : a + b + c;
}


int t3()
{
    if (1) { return 5; }
    if (0) { return 2; }
    if (1) { return 3; } else { return 4; }
    if (0) { return 5; } else { return 6; }

    int a = 9;
    if (a == 8);
    if (a == 10) { putchar(97); }
    if (a == 7) { return 7; }
    else if (a == 2) { return 8; }
    else if (a == 5) { return 9; }
    else { return 0; }

    if (a == 23) putchar(107);
    if (a == 79) putchar(202);
    else         putchar(203);
}


void t4()
{
    int a = 10;
l1:
    if (a == 91) { goto l1; }

l2:
    if (a >= 0) { a -= 1; goto l2; }

l3:
    a = 10;
    if (a >= 0)
    {
        int b = 5;
        if (a <= 20) { goto l4; }
l4:
        if (a == 10) { goto l5; }
        else { goto l3; }
    }

l5: l6:
    goto l4;
}


int t5()
{
    switch (8)
    {
    case 8: return 1;
    case 7: break;
    case 6: break;
    case 5: break;
    }

    switch (8 + 8 * 9)
    {
    case 8: return 1;
    case 7: break;
    case 6: break;
    case 5: break;
    }

    switch (2)
    {
    case 3: break;
    case 4: break;
    }

    return 0;
}


int t6()
{
    int a = 5;
    switch (a)
    {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    default: break;
    }

    switch (a + 2)
    {
    case 1: putchar(97); break;
    case 2: putchar(98);
    case 3: putchar(99);
    default: putchar(100);
    case 4: putchar(101); break;
    }

    return 0;
}

void t7()
{
    int a = 9;
    switch (a)
    {
    case 8:
        switch (a + 20)
        {
        case 20:
            break;
        default:
            break;
        case 30:
            switch (a + 30)
            {
            case 200:
                break;
            case 300:
                putchar(20);
                putchar(70);
            case 500:
                putchar(600);
            default:
                break;
            }
        }
    case 20:
        putchar(97);
    }
}

void t8()
{
    int a = 10;
    goto l1;
    switch (a + 10 - a * 45)
    {
    case 20: break;
l1:
    case 21: break;
    case 22: goto l1;
l2:
    case 23: goto l2;
    case 24: putchar(102);
    }

    putchar(103);
}


int main(void)
{
    t1();
    t2();
    t3();
    t4();
    t5();
    t6();
    t7();
}
