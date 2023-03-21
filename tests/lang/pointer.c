void t1()
{
    int* a = 0;
    char* b = 0;
    char**** c = 0;
    char* const * const * d = 0;
    char* const restrict * e;
    restrict const char* g = 0;
}

void t2()
{
    int a = 20;
    int* pa = &a;
    *pa = 40;
}

void t3()
{
    long a = 123456789;
    long* p = &a;
    p += 2;
    p -= 3;
    p = p + 3;
    p = p - 3;
}

void t4()
{
    long a = 123456789;
    long* p = &a;
    long* pp = &(*p);

    p[2] = 20;
    p[3] = 40;
    *(p + 30) = 20;
    (p + 10)[20] = 0;
}

void t5(int* a, int* b)
{

}

int func()
{
    return 0;
}

int func2(int a, int b, int c)
{
    return a + b + c;
}

int (*func3(int a, int b, int c))()
{
    int d = a + b + c;
    return func;
}

int (*func4(int a, int b, int c))(int, int, int)
{
    int (*pfunc)(int, int, int) = func2;
    pfunc(a, b, c);
    return pfunc;
}

int (*func5(int a, int b, int c))(int a, int b, int c)
{
    int (*pfunc)(int, int, int) = func2;
    return pfunc;
}

int (*(*func6())(int, int, int))(int, int, int)
{
    return func5;
}

int main()
{
    t1();
    t2();
    t3();
    t4();

    int a = 0;
    int b = 0;
    t5(&a, &b);
}
