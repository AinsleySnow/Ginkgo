static int func(int a)
{
    return a * 2;
}

static int cc = 20;
constexpr int c = 30;

extern int b = 100;

// If the declaration of an identifier for an object has
// file scope and no storage-class specifier or only the
// specifier auto, its linkage is external.
int d = 40;
auto int e = 50;

int func2(int a)
{
    return a + 3;
}
