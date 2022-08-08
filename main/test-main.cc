#include <cassert>
#include <cstdio>
#include <cstring>
#include <array>
#include <functional>
#include <memory>
#include <string>

#include "../front/Node.h"
#include "../front/parser.hh"
#include "../utilities/SymbolTable.h"

#ifdef YYDEBUG
extern int yydebug;
#endif

extern FILE* yyin;
extern SymbolTable GlobalSymbols;


bool test_speccomb()
{
    printf("spec-combination.c: ");
    yyin = fopen(files[0].first.c_str(), "r");
    assert(yyin);

    std::unique_ptr<Node> temp{};
    yyparse(temp);
    GlobalSymbols.PrintSymbols();

    return true;
}


using pair = std::pair<std::string, std::function<bool(void)>>;
using pair_array = std::array<pair, 1>;

static const pair_array files{
    pair("main/test-cases/spec-combination.c", test_speccomb)
};


int main(void)
{
#ifdef YYDEBUG
    yydebug = 1;
#endif

    for (pair func : files)
    {
        try
        {
            if (func.second())
                printf("ok\n");
            else
                printf("error\n");
        }
        catch (const std::exception& e)
        {
            printf("exception thrown\n");
        }
        catch (int e)
        {
            printf("%d\n", e);
        }
    }
    return 0;
}
