int main()
{
    char* empty = "";
    char* a = "hello";
    char* b = u8"你好";
    short* c = u"你好";
    int* d = U"你好";
    int* e = L"你好";

    char* f = "hello " "how " "are " "you ";
    char* g = u8"fine " u8"thanks " u8"and " u8"you ";

    // from https://www.zhihu.com/question/315818061/answer/2654150452
    short* h = u"你" u"来自" u"哪里";
    int* i = U"我" U"来自" U"省会石家庄";
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
}
