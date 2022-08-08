#include <cassert>
#include <cstdio>
#include <cstring>
#include <array>
#include <functional>
#include <memory>
#include <string>

#include "../front/Node.h"
#include "../utilities/SymbolTable.h"

#include "../front/parser.hh"

#ifdef YYDEBUG
extern int yydebug;
#endif

extern FILE* yyin;

using pair = std::pair<std::string, std::function<bool(void)>>;
using pair_array = std::array<pair, 1>;

bool test_speccomb();

static const pair_array files{
    pair("main/test-cases/spec-combination.c", test_speccomb)
};


bool test_speccomb()
{
    printf("spec-combination.c: ");
    yyin = fopen(files[0].first.c_str(), "r");
    assert(yyin);

    SymbolTable globalSymbols{};
    yyparse(globalSymbols);
    globalSymbols.PrintSymbols();

    return true;
}


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
