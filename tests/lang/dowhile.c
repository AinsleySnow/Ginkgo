#include "test.h"

void constant()
{
    do ; while (0);
    do assert(1); while (0);
    do { assert(1); } while (0);
}

void common()
{
    int i = 97;
    do
    {
        i += 1;
    } while (i < 107);
    assert(i == 107);
}

void continue_break1()
{
    int i = 0;
    int a = 0;

    do {
        i++;
        if (i == 3)
            break;
        a += i;
    } while (i < 5);
    assert(i == 3);
    assert(a == 3);

    i = 0;
    do {
        i++;
        if (i == 3)
            continue;
        a += i;
    } while (i < 5);
    assert(i == 5);
    assert(a == 15);
}

void continue_break2()
{
    int i = 0, j = 0;
    int a = 0;

    do {
        i++;
        j = 0;
        do {
            j++;
            if (j == 3)
                break;
            a += i * j;
        } while (j < 5);
    } while (i < 3);
    assert(i == 3 && j == 3);
    assert(a == 18);

    i = 0, j = 0, a = 0;
    do {
        i++;
        j = 0;
        do {
            j++;
            if (j == 3)
                continue;
            a += i * j;
        } while (j < 5);
    } while (i < 3);
    assert(i == 3 && j == 5);
    assert(a == 72);
}

void nest()
{
    int i = 1, j = 1, k = 1;
    int a = 0;
    do
    {
        do
        {
            do
            {
                a += i * j * k;
                k += 1;
            } while (k <= 9);
            j += 1;
        } while (j <= 9);
        i += 1;
    } while (i <= 9);
    assert(a == 15153);
}

void with_goto()
{
    int i = 97, j = 20, k = 80;
    do
    {
        do
        {
            i += 1;
            goto l1;
        } while (i < 107);
        i += 1;
    } while (i < 107);

l1:
    assert(i == 98);
    do
    {
l3:
        ;
        do
        {
            j = 30;
l2:
            if (j >= 35)
                goto l4;
            do
            {
                j = 35;
                if (j >= 35)
                    goto l2;
            } while (k < 107);
            goto l3;
        } while (j < 107);
l4:
        i += 30;
    } while (i < 107);

    assert(i == 128);
    assert(j == 35);
}

void combine()
{
    int i = 1, j = 1, k = 1;
    int a = 20;
    for (; i <= 10; ++i)
    {
        while (j <= 10)
        {
            j += 1;
            do
            {
                a += i * j * k;
                k += 1;
            } while (k <= 10);
        }
    }
    assert(a == 1135);

    while (j <= 20)
    {
        j += 1;
        for (; i <= 20; ++i)
        {
            do
            {
                a += i * j * k;
                k += 1;
            } while (k <= 20);
        }
    }
    assert(a == 47695);

    do
    {
        while (j <= 30)
        {
            j += 1;
            for (; i < 30; ++i)
                a += i * j * k;
        }
        k += 1;
    } while (k <= 30);
    assert(a == 196195);
}

int main()
{
    constant();
    common();
    continue_break1();
    continue_break2();
    nest();
    with_goto();
    combine();

    SUCCESS;
}
