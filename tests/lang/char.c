#include "test.h"

char a = '1';
int b = L'你';
int c = u'好';
int d = U'吗';

char e = '\'';
char f = '\"';
char g = '"';
char h = '\\';
char i = '\?';
char j = '\141';
char k = '\x61';

char l = '\a';
char m = '\b';
char n = '\f';
char o = '\n';
char p = '\r';
char q = '\t';
char r = '\v';

char s = u'\'';
char t = U'\'';
char u = L'\'';


int main()
{
    assert(a == 49);    assert(b == 20320);
    assert(c == 22909); assert(d == 21527);
    assert(e == 39);    assert(f == 34);
    assert(g == 34);    assert(h == 92);
    assert(i == 63);    assert(j == 97);
    assert(k == 97);    assert(l == 7);
    assert(m == 8);     assert(n == 12);
    assert(o == 10);    assert(p == 13);
    assert(q == 9);     assert(r == 11);

    SUCCESS;
}
