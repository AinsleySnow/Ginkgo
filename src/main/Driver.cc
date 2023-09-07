#include "main/Driver.h"
#include "pass/FlowGraph.h"
#include "pass/DUInfo.h"
#include "pass/Liveness.h"
#include "pass/LoopAnalyze.h"
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
#include <iostream>

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
        libpath_ = "/usr/lib/Ginkgo/gkcommon.a";
        libc23path_ = "/usr/lib/Ginkgo/gkc23.a";
        includepath_ = "/usr/include/Ginkgo";
    }
    else
    {
        auto path = absolute.string();
        cppath_ = path + "gkcpp";
        libpath_ = path + "../lib/gkcommon.a";
        libc23path_ = path + "../lib/gkc23.a";
        includepath_ = path + "../../include";
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
    // -I: add a directory to the search list of gkcpp
    system(fmt::format("{} -V -H -b -I{} {} > {}",
        cppath_, includepath_, input, afterpp).c_str());
    return afterpp;
}

void Driver::Parse(const std::string& name)
{
    yyin = fopen(name.c_str(), "r");
    yy::parser parser(transunit_, CheckType());
    // parser.set_debug_level(1);
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
    simple.AddPass<FlowGraph>(100);
    simple.AddPass<DUInfo>(200);
    simple.AddPass<LoopAnalyze>(300, 100);
    simple.AddPass<Liveness>(400, 100, 200, 300);
    simple.AddPass<SimpleAlloc>(500, 200, 400);
    return std::move(simple);
}

void Driver::GenerateAsm(const std::string& output)
{
    // Note here that the parameter stands for output
    // file name, not input as in the other methods.
    Pipeline pl = InitPipeline();
    CodeGen codegen{ output, &pl, pl.GetPass<SimpleAlloc>(500) };

    std::ostream* pstream = nullptr;
    if (summaryflag_)
    {
        if (passtream_.empty())
            pstream = &std::cout;
        else
            pstream = new std::ofstream(passtream_);
        codegen.SetSummaryStream(pstream);

        for (auto i : modpassprint_)
            codegen.AddModulePass2Print(pl.GetPass<ModulePass>(i));
        for (auto i : funcpassprint_)
            codegen.AddFuncPass2Print(pl.GetPass<FunctionPass>(i));
    }

    codegen.VisitModule(module_.get());
    if (!passtream_.empty())
        delete pstream;
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
        "{} "
        "{} ",
        outputname_, input, libpath_);

    if (link2gk_)
        basic += libc23path_ + ' ';

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
