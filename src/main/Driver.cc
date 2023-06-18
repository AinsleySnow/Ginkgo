#include "main/Driver.h"
#include "pass/SimpleAlloc.h"
#include "visitast/CodeChk.h"
#include "visitast/IRGen.h"
#include "visitir/CodeGen.h"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <random>

extern FILE* yyin;


static std::string StripExtension(const std::string& name)
{
    auto index = name.find_last_of('.');
    if (index == std::string::npos)
        return name;
    return name.substr(0, index);
}

Driver::Driver(OutputType ty, const char* e, const std::string& in, const std::string& out) :
    outputype_(ty), environment_(e), inputname_(in), afterpp_(in)
{
    if (!out.empty())
    {
        outputname_ = std::move(out);
        return;
    }

    if (outputype_ == OutputType::assembly)
        outputname_ = StripExtension(inputname_) + ".s";
    else if (outputype_ == OutputType::intermediate)
        outputname_ = StripExtension(inputname_) + ".ll";
    else
        outputname_ = "a.out";
}


void Driver::Preprocess()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0x7FFFFFFF);
    auto temp = std::filesystem::temp_directory_path() / (std::to_string(dis(gen)) + ".c");

    // this is the case for not installing ginkgo
    // the ginkgo itself is in the same directory with gkcpp
    auto absolute = (std::filesystem::current_path() / environment_).remove_filename();
    // otherwise... Is Ginkgo already installed?
    if (!std::filesystem::exists(absolute / "gkcpp"))
        absolute = "gkcpp";
    else
        absolute.append("gkcpp");

    // -V: not showing version information
    // -H: output blank lines
    // -b: output unbalanced braces, brackets, etc.
    system(fmt::format("{} -V -H -b {} > {}",
        absolute.string(), inputname_, temp.string()).c_str());
    afterpp_ = std::move(temp);
}

void Driver::Parse()
{
    yyin = fopen(afterpp_.c_str(), "r");
    yy::parser parser(transunit_, CheckType());
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


void Driver::Compile()
{
    Preprocess();
    Parse();
    GenerateIR();
}

void Driver::EmitBinary()
{
    EmitAssembly();
    system(fmt::format("as {}.s -o {}.o",
        StripExtension(inputname_), StripExtension(outputname_)).c_str());
    system(fmt::format("ld -o {} "
            "-dynamic-linker /lib64/ld-linux-x86-64.so.2 "
            "/usr/lib/x86_64-linux-gnu/crti.o "
            "/usr/lib/x86_64-linux-gnu/crt1.o "
            "-lc "
            "{}.o "
            "/usr/lib/x86_64-linux-gnu/crtn.o",
            outputname_, outputname_).c_str());
}

void Driver::EmitAssembly()
{
    SimpleAlloc alloc{ module_.get() };
    alloc.Execute();
    CodeGen codegen{ outputname_, alloc };
    codegen.VisitModule(module_.get());
}

void Driver::EmitIntermediate()
{
    auto output = std::ofstream(outputname_);
    output << module_->ToString();
    output.close();
}
