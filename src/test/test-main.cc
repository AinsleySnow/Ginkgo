#include <cassert>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "ast/Expr.h"
#include "ast/Identifier.h"
#include "ast/Tag.h"
#include "types/Type.h"
#include "utils/Scope.h"

#define YYDEBUG
#include "parser/yacc.hh"

bool debug{};
extern FILE* yyin;
using map = std::map<std::string, std::function<bool(const Scope&)>>;
static const std::string path{ "tests/test-cases/" };
static const std::string extend{ ".c" };


bool test_evaluate(const Scope&);
bool test_speccomb(const Scope&);
bool test_speclist(const Scope&);
bool test_qualspec(const Scope&);

static const map files{
    {"evaluate", test_evaluate},
    {"spec-combination", test_speccomb},
    {"spec-list", test_speclist},
    {"qual-spec", test_qualspec}
};


bool run(const std::string& name,
    const std::function<bool(const Scope&)>& func)
{
    try
    {
        yyin = fopen((path + name + extend).c_str(), "r");
        assert(yyin != nullptr);

        Scope scope{ Scope::ScopeType::file };
        yy::parser p(&scope);
        
        #ifdef YYDEBUG
        if (debug)
            p.set_debug_level(1);
        #endif
        
        p.parse();

        printf("%s: ", name.c_str());
        if (func(scope))
        {
           printf("ok\n");
           return true;
        }
        else
        {
            printf("error\n");
            return false;
        }
    }
    catch (const std::exception& e)
    {
        printf("exception thrown\n");
        return false;
    }
    catch (int e)
    {
        printf("An exception has been thrown by the front end: %d\n", e);
        return false;
    }
}


int main(int argc, char** args)
{
    std::string justone{};
    if (argc > 3 && (argc != 5 && argc != 4))
    {
        printf("usage: test [-t] [<case-name>] [-o <file>]\n");
        return argc;
    }
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp("-t", args[i]) == 0)
        {
            #ifdef YYDEBUG
            debug = true;
            #else
            printf("YYDEBUG not defined. Ignoring \"-t\".\n");
            #endif
        }
        else if (strcmp("-o", args[i]) == 0)
        {
            if (i + 1 >= argc)
            {
                printf("usage: test [-t] [<case-name>] [-o <file>]\n");
                return -1;
            }
            freopen(args[i + 1], "w+", stderr);
            i += 1;
        }
        else
            justone = args[i];
    }

    if (!justone.empty())
    {
        run(justone, files.at(justone));
        return 0;
    }

    int pass = 0, fail = 0;
    for (auto pair : files)
    {
        if (run(pair.first, pair.second))
            ++pass;
        else
            ++fail;
    }
    if (fail)
        printf("Tested %d case(s) -- %d passed, %d failed.\n", pass + fail, pass, fail);
    else
        printf("All cases passed!\n");
    return 0;
}
