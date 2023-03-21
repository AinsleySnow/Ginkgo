void t1()
{
    int a[4];
    a[3] = 10;
    a[0] = 0;
}

void t2()
{
    int a[4];
    int* b = a;
    *b = 3;
    *(b + 2) = 4;
    b[2] = 5;
}

void t3()
{
    int a[4][5];
    a[2][3] = 10;
    a[2][1] += 20;
    int c = a[1][0] + 30;
    c = a[1][2] * 20;
}

void t4()
{
    int a[3];
    int (*pa)[3] = a;
    (*pa)[1] = 3;
}

int (*t5())[3]
{
    int b[3];
    return &b;
}

void t6()
{
    int a[4];
    *a = 4;
    *(a + 2) = 10;
}

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int mul(int a, int b)
{
    return a * b;
}

int div(int a, int b)
{
    return a / b;
}

void t7()
{
    int a = 20, b = 30;

    int (*array[4])(int, int);
    array[0] = add;
    array[1] = sub;
    array[2] = mul;
    array[3] = div;

    array[0](a, b);
    array[1](a, b);
    array[2](a, b);
    array[3](a, b);

    (*array)(a, b);
    (*(array + 1))(a, b);
    (*(array + 2))(a, b);
    (*(array + 3))(a, b);
}

void t8(int a[], int b[4])
{

}
