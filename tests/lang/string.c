int main()
{
    char* empty = "";
    char* a = "hello";
    char* b = u8"你好";
    char* c = u"你好";
    char* d = U"你好";
    char* e = L"你好";

    char* f = "hello" "how" "are" "you";
    char* g = u8"fine" u8"thanks" u8"and" u8"you";

    // from https://www.zhihu.com/question/315818061/answer/2654150452
    char* h = u"你" u"来自" u"哪里";
    char* i = U"我" U"来自" U"省会石家庄";
    char* j = L"石家庄" L"正宗" L"安徽" L"牛肉板面";

    // How differently-perfixed wide string will be processed
    // was defined by implementation in C11 but in C23 the
    // string will be treated as a character string literal.
    // 6.4.5[5] in both C11 and C23 standard
    char* k = u"棋" U"子" L"烧" u"饼";
}
