void exit(int);

int add(int, int), sub(int x, int);
double mul(double a, double b);
int many_int_argv(int, int, int, int, int, int, int, int, int, int, int, int, int, int);
float many_float_argv(float, float, float, float, float, float, float, float, float, float, float, float, float, float);
void _void(void);
void _void2();
int _void3();

inline int add_up(int a, int b, int c)
{
    return a + b + c;
}

_Noreturn void no_return(int a)
{
    exit(a);
}

int main(void)
{
    int putchar(int);
    putchar(2);
    int sum = add(2, 3);
    double product = mul(4, 9);
    int diff = sub(4, 202);
    int sum2 = many_argv(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
    _void();
    no_return(3);
    // 'return 0' will be automatically generated
}

int add(int a, int b)
{
    return a + b;
}

double mul(double a, double b)
{
    return a * b;
}

int sub(int a, int)
{
    return a - 8;
}

int many_int_argv(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n)
{
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

float many_float_argv(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n)
{
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

void _void(void)
{
}

void _void2(void)
{
    ;;;
}

int _void3()
{
    return 0;
}
