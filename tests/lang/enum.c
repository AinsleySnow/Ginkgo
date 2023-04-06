int putchar(int);

enum e
{
    E_A, E_B, E_C
};

enum f
{
    F_A = 2,
    F_B,
    F_C = 20,
    F_D = F_B + 1
};

enum g : unsigned long
{
    G_A = 4294967296,
    G_B,
    G_C
};

enum
{
    H_A,
    H_B,
    H_C
};

void t1()
{
    int a = E_A + E_B + E_C;
    int b = F_A + F_B + F_C + F_D;
    unsigned long c = G_A + G_B + G_C;
}

void t2()
{
    enum { t2_1, t2_2 } a = t2_1;
    putchar(a);
}

void t3()
{
    enum t3_e { t3_1, t3_2 } a = t3_1;
    putchar(a);
    enum t3_e c = t3_2;
    putchar(c);
}
