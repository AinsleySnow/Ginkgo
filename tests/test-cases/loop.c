int putchar(int);

void t1()
{
    for (; 1; )    putchar(97);
    for (; 0; )    putchar(97);
    for (1; 0; )   putchar(3);
    for (0; 0; )   putchar(5);
    for (1; 1; )   putchar(6);
    for (0; 1; )   putchar(7);
    for (int; 1; ) putchar(97);

    for (int i = 97; 0; i++) putchar(i);
    for (int i = 97; 1; i++) putchar(i);
    for (int i = 97; 1; i++);
}

void t2()
{
    for (int i = 97; i < 107; ++i)
        putchar(i);

    int i = 97;
    for (; i < 107; ++i)
        putchar(i);

    for (;; ++i)
    {
        if (i >= 107) break;
        putchar(i);
    }

    for (;;)
    {
        if (i >= 107) break;
        putchar(i++);
    }

    for (int i = 97; i < 107; ++i)
    {
        if (i == 102) continue;
        putchar(i);
    }
}


void t3()
{
    for (int i = 49; i <= 58; ++i)
    {
        for (int j = 49; j <= 58; ++j)
        {
            putchar(i);
            putchar(j);
        }
    }

    for (int i = 49; i < 58; ++i)
    {
        for (int j = 49; j <= 58; ++j)
        {
            if (j == 57) break;
            if (j == 51) continue;
            putchar(i);
            putchar(j);
        }

        if (i == 57) break;
        if (i == 51) continue;
    }
}


void t4()
{
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            for (int k = 0; k < 10; ++k)
                goto l1;
l1:

    for (int i = 0; i < 10; ++i)
    {
l3:
        for (int j = 0; j < 10; ++j)
        {
l2:
            for (int k = 0; k < 10; ++k)
                goto l2;
            goto l3;
        }
    }
}


void t5()
{
    while (0);
    while (1);
    while (1) putchar(97);
    while (0) putchar(97);
}

void t6()
{
    int i = 97;
    while (i < 107)
    {
        putchar(i);
        i += 1;
    }
}

void t7()
{
    int i = 97;
    while (i < 107)
    {
        putchar(i);
        while (i < 107)
        {
            while (i != 106) continue;
            break;
        }
        break;
    }
}

void t8()
{
    int i = 97;
    while (i < 107)
        while (i < 107)
            goto l1;

l1:
    while (i < 97)
    {
l3:
        ; // Standard C11 grammar seems not support to
        // directly jump to a declaration.
        // I'll fix it in some further commit.
        int j = 97;
        while (j < 107)
        {
l2:
            ;
            int k = 97;
            while (k < 107)
                goto l2;
            goto l3;
        }
    }
}


void t9()
{
    do while (0);
    do while (1);
    do { putchar(97); } while (0);
    do { putchar(97); } while (1);
}

void t10()
{
    int i = 97;
    do
    {
        putchar(i);
        i += 1;
    } while (i < 107);
}

void t11()
{
    int i = 97;
    do
    {
        putchar(i);
        do
        {
            do
            {
                continue;
            } while (i < 107);
            break;
        } while (i < 107);
        i += 1;
        break;
    } while (i < 107);
}


void t12()
{
    int i = 97;
    do
    {
        do
        {
            goto l1;
        } while (i < 107);
    } while (i < 107);

l1:
    do
    {
l3:
        ;
        int j = 97;
        do
        {
l2:
            int k = 97;
            do
            {
                goto l2;
            } while (k < 107);
            goto l3;
        } while (j < 107);
    } while (i < 107);
}


void t13()
{
    int i = 0, j = 0, k = 0;
    for (; i < 10; ++i)
    {
        while (j < 10)
        {
            j += 1;
            do
            {
                k += 1;
            } while (k < 10);
        }
    }

    i = 0, j = 0, k = 0;
    while (j < 10)
    {
        j += 1;
        for (; i < 10; ++i)
        {
            do
            {
                k += 1;
            } while (k < 10);
        }
    }

    i = 0, j = 0, k = 0;
    do
    {
        while (j < 10)
        {
            j += 1;
            for (; i < 10; ++i)
                k += 1;
        }
    } while (k < 10);    
}


int main()
{
    t1(); t2(); t3(); t4(); t5();
    t6(); t7(); t8(); t9(); t10();
    t11(); t12(); t13();
}
