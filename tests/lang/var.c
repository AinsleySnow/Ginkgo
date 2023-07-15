#include "test.h"

void size()
{
    char a;
    unsigned char b;
    signed char c;

    assert(sizeof(a) == 1);
    assert(sizeof(b) == 1);
    assert(sizeof(c) == 1);

    short d;
    short int e;
    unsigned short f;
    signed short g;
    unsigned short int h;
    signed short int i;

    assert(sizeof(d) == 2); assert(sizeof(e) == 2);
    assert(sizeof(f) == 2); assert(sizeof(g) == 2);
    assert(sizeof(h) == 2); assert(sizeof(i) == 2);

    int j;
    signed k;
    unsigned l;
    signed int m;
    unsigned int n;

    assert(sizeof(j) == 4); assert(sizeof(k) == 4);
    assert(sizeof(l) == 4); assert(sizeof(m) == 4);
    assert(sizeof(n) == 4);

    long o;
    unsigned long p;
    signed long q;
    long long r;
    unsigned long long s;
    signed long long t;
    long int u;
    unsigned long int v;
    signed long int w;
    long long int x;
    unsigned long long int y;
    signed long long int z;

    assert(sizeof(o) == 8); assert(sizeof(p) == 8);
    assert(sizeof(q) == 8); assert(sizeof(r) == 8);
    assert(sizeof(s) == 8); assert(sizeof(t) == 8);
    assert(sizeof(u) == 8); assert(sizeof(v) == 8);
    assert(sizeof(w) == 8); assert(sizeof(x) == 8);
    assert(sizeof(y) == 8); assert(sizeof(z) == 8);

    float aa;
    double ab;
    long double ac;

    assert(sizeof(aa) == 4);
    assert(sizeof(ab) == 8);
    // size of long double should be 16.
    // assert(sizeof(ac) == 8);
}

void many()
{
    int ad, ae, af;
    assert(sizeof(ad) == 4);
    assert(sizeof(ae) == 4);
    assert(sizeof(af) == 4);

    char ag, ah, ai, aj;
    assert(sizeof(ag) == 1); assert(sizeof(ah) == 1);
    assert(sizeof(ai) == 1); assert(sizeof(aj) == 1);

    long al, am, an, ao, ap;
    assert(sizeof(al) == 8); assert(sizeof(am) == 8);
    assert(sizeof(an) == 8); assert(sizeof(ao) == 8);
    assert(sizeof(ap) == 8);

    short aq, ar, as, at, au, av;
    assert(sizeof(aq) == 2); assert(sizeof(ar) == 2);
    assert(sizeof(as) == 2); assert(sizeof(at) == 2);
    assert(sizeof(au) == 2); assert(sizeof(av) == 2);

    double aw, ax, ay, az;
    assert(sizeof(aw) == 8); assert(sizeof(ax) == 8);
    assert(sizeof(ay) == 8); assert(sizeof(az) == 8);

    float ba, bb, bc, bd;
    assert(sizeof(ba) == 4); assert(sizeof(bb) == 4);
    assert(sizeof(bc) == 4); assert(sizeof(bd) == 4);
}

int main(void)
{
    size();
    many();
    SUCCESS;
}
