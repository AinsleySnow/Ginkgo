#include "main/Driver.h"
#include "visitast/CodeChk.h"
#include "visitast/IRGen.h"
#include <cstdio>


extern FILE* yyin;

void Driver::Parse()
{
    yyin = fopen(inputname_.c_str(), "r");
    yy::parser parser(transunit_);
    parser.parse();
}

const auto& Driver::GetAST()
{
    return transunit_;
}

bool Driver::CheckAST()
{
    CodeChk codechk = CodeChk();
    codechk.VisitTransUnit(&transunit_);
    return true;
}

void Driver::GenerateIR()
{
    if (!CheckAST())
        return;
    IRGen irgen = IRGen(inputname_);
    irgen.VisitTransUnit(&transunit_);
    module_ = std::move(irgen.GetModule());
}

void Driver::PrintIR()
{
    std::cout << module_->ToString();
}
