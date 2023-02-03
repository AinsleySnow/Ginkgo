#include "main/Driver.h"
#include "visitast/IRGen.h"


Driver::Driver(const std::string& in) :
    inputname_(in)
{
    input_.open(in);
}

Driver::Driver(const std::string& in, const std::string& out) :
    inputname_(in), outputname_(out)
{
    input_.open(in);
    output_.open(out);
}

Driver::~Driver()
{
    if (input_.is_open()) input_.close();
    if (output_.is_open()) output_.close();
}

void Driver::Parse()
{
    yy::parser parser(transunits_);
    parser.parse();
}

const auto& Driver::GetAST()
{
    return transunits_;
}

void Driver::CheckAST()
{

}

void Driver::GenerateIR()
{
    IRGen irgen = IRGen(inputname_);
    for (auto& pdecl : transunits_)
        irgen.VisitTransUnit(pdecl.get());
    module_ = std::move(irgen.GetModule());
}
