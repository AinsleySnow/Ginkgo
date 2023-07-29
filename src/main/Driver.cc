#include "main/Driver.h"
#include "pass/DUInfo.h"
#include "pass/Pipeline.h"
#include "pass/SimpleAlloc.h"
#include "parser/yacc.hh"
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

Driver::Driver(const char* e)
{
    // find path of gkcpp and gklib.a
    // this is the case for not installing ginkgo
    // the ginkgo itself is in the same directory with gkcpp
    auto absolute = (std::filesystem::current_path() / e).remove_filename();
    // otherwise... Is Ginkgo already installed?
    if (!std::filesystem::exists(absolute / "gkcpp"))
    {
        cppath_ = "gkcpp";
        libpath_ = "/usr/lib/Ginkgo/gklib.a";
    }
    else
    {
        absolute.append("gkcpp");
        cppath_ = absolute.string();
        absolute.remove_filename();
        absolute.append("../lib/gklib.a");
        libpath_ = absolute.string();
    }
}


std::string Driver::GetRandom() const
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 0x7FFFFFFF);
    return std::to_string(dis(gen));
}

std::string Driver::Path2Temp(const std::string& name, char ext) const
{
    return fmt::format("{}.{}",
        (std::filesystem::temp_directory_path() / name).string(), ext);
}

void Driver::Prologue()
{
    switch (outputype_)
    {
    case OutputType::binary:
        if (outputname_.empty())
            outputname_ = "a.out";
        break;
    case OutputType::intermediate:
        if (outputname_.empty())
            outputname_ = StripExtension(inputname_) + ".ll";
        break;
    case OutputType::assembly:
        if (outputname_.empty())
            outputname_ = StripExtension(inputname_) + ".s";
        break;
    }
}

std::string Driver::Preprocess(const std::string& input)
{
    auto afterpp = Path2Temp(GetRandom(), 'c');
    // -V: not showing version information
    // -H: output blank lines
    // -b: output unbalanced braces, brackets, etc.
    system(fmt::format("{} -V -H -b {} > {}",
        cppath_, input, afterpp).c_str());
    return afterpp;
}

void Driver::Parse(const std::string& name)
{
    yyin = fopen(name.c_str(), "r");
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

Pipeline Driver::InitPipeline()
{
    Pipeline simple{ module_.get() };
    simple.AddPass<DUInfo>(0);
    simple.AddPass<SimpleAlloc>(1, 0);
    return std::move(simple);
}

void Driver::GenerateAsm(const std::string& output)
{
    // Note here that the parameter stands for output
    // file name, not input as in the other methods.
    Pipeline pl = InitPipeline();
    pl.ExecuteAll();
    CodeGen codegen{ output, pl.GetPass<SimpleAlloc>(1) };
    codegen.VisitModule(module_.get());
}

std::string Driver::Assemble(const std::string& input)
{
    auto output = Path2Temp(GetRandom(), 'o');
    system(fmt::format("as {} -o {}", input, output).c_str());
    return output;
}

void Driver::Link(const std::string& input)
{
    std::string basic = fmt::format(
        "ld -o {} "
        "-dynamic-linker /lib64/ld-linux-x86-64.so.2 "
        "/usr/lib/x86_64-linux-gnu/crt1.o "
        "/usr/lib/x86_64-linux-gnu/crti.o "
        "-lc "
        "{} ",
        outputname_, input);

    if (link2gk_)
        basic += libpath_ + ' ';

    system(fmt::format(
            "{} "
            "/usr/lib/x86_64-linux-gnu/crtn.o",
            basic).c_str());
}


std::string Driver::Compile()
{
    Prologue();
    auto afterpp = Preprocess(inputname_);
    Parse(afterpp);
    GenerateIR();
    return afterpp;
}

void Driver::EmitBinary()
{
    auto afterpp = Compile();
    auto assembly = Path2Temp(GetRandom(), 's');
    GenerateAsm(assembly);
    auto obj = Assemble(assembly);
    Link(obj);
}

void Driver::EmitAssembly()
{
    Compile();
    GenerateAsm(outputname_);
}

void Driver::EmitIntermediate()
{
    Compile();
    auto output = std::ofstream(outputname_);
    output << module_->ToString();
    output.close();
}


void Driver::Run()
{
    if (outputype_ == OutputType::binary)
        EmitBinary();
    else if (outputype_ == OutputType::assembly)
        EmitAssembly();
    else if (outputype_ == OutputType::intermediate)
        EmitIntermediate();
}
