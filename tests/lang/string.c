#include "test.h"

int main()
{
    char* empty = "";
    char* a = "hello";
    unsigned char* b = u8"你好";
    unsigned short* c = u"你好";
    unsigned int* d = U"你好";
    unsigned int* o = U"hello";
    int* e = L"你好";

    char* f = "hello " "how " "are " "you ";
    unsigned char* g = u8"fine " u8"thanks " u8"and " u8"you ";

    // from https://www.zhihu.com/question/315818061/answer/2654150452
    unsigned short* h = u"你" u"来自" u"哪里";
    unsigned int* i = U"我" U"来自" U"省会石家庄";
    int* j = L"石家庄" L"正宗" L"安徽" L"牛肉板面";

    // How differently-perfixed wide string will be processed
    // was defined by implementation in C11 but in C23 the
    // string will be treated as a character string literal.
    // 6.4.5[5] in both C11 and C23 standard
    char* k = u"棋" U"子" L"烧" u"饼";

    char* l = "\'\"\"'" "\"\\\\\"'''\"" "\'\'\' \"";
    char* m = "\xe6\xb8\xa4\xe6\xb5\xb7\xe9\xad\x82"
              "\xe7\x87\x95\xe5\xb1\xb1\xe9\xa3\x8e"
              "\xe5\x87\xa4\xe5\x87\xb0\xe5\x9f\x8e";
    char* n = "\a\b\f\n\r\t\v\?";

    assert(a[0] == 'h');        assert(a[4] == 'o');
    assert(b[0] == 0xe4);       assert(b[5] == 0xbd);
    assert(c[0] == 0x4f60);     assert(c[1] == 0x597d);
    assert(d[0] == 0x4f60);     assert(d[1] == 0x597d);
    assert(e[0] == 0x4f60);     assert(e[1] == 0x597d);
    assert(f[0] == 'h');        assert(f[16] == 'u');
    assert(g[0] == 'f');        assert(g[18] == 'u');
    assert(h[0] == 0x4f60);     assert(h[4] == 0x91cc);
    assert(i[0] == 0x6211);     assert(i[7] == 0x5e84);
    assert(j[0] == 0x77f3);     assert(j[10] == 0x9762);
    // assert(k[0] == (char)0xe6);       assert(k[11] == (char)0xbc);
    assert(l[0] == '\'');       assert(l[16] == '"');
    // assert(m[0] == (char)0xe6);       assert(m[27] == (char)0x8e);
    assert(n[0] == '\a');       assert(n[7] == '?');

    SUCCESS;
}
