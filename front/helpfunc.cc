#include "parser.hh"
#include <string>
#include <queue>

extern char* yytext;

std::queue<std::string> symbols {}; 

extern "C" void save_symbol()
{
    std::string literal { yytext };
    symbols.push(literal);
}
