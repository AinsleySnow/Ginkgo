#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "ast/Statement.h"
#include "IR/Value.h"
#include <memory>
#include <string>
#include <vector>

class Pipeline;


enum class OutputType
{
    binary,
    assembly,
    intermediate
};

class Driver
{
public:
    Driver(const char* e);

    void AddIncludeDir(const std::string& d) { extrainclude_ += "-I" + d + ' '; }
    void SetOutputType(OutputType ty) { outputype_ = ty; }
    void SetLink2Ginkgo(bool l) { link2gk_ = l; }
    void SetInputName(const std::string& n) { inputname_ = n; }
    void SetOutputName(const std::string& n) { outputname_ = n; }

    void SetSummaryFlag() { summaryflag_ = true; }
    void SetSummaryStream(const std::string& o) { passtream_ = o; }
    void AddModulePass2Print(int p) { modpassprint_.push_back(p); }
    void AddFuncPass2Print(int p) { funcpassprint_.push_back(p); }

    void Run();

private:
    std::string GetRandom() const;
    std::string Path2Temp(const std::string&, char) const;

    void Prologue();
    std::string Preprocess(const std::string&);
    void Parse(const std::string&);
    std::string Compile();
    const auto& GetAST();
    bool CheckAST();
    void GenerateIR();
    Pipeline InitPipeline();
    void GenerateAsm(const std::string&);
    std::string Assemble(const std::string&);
    void Link(const std::string&);

    void EmitBinary();
    void EmitAssembly();
    void EmitIntermediate();

    OutputType outputype_{};
    std::string cppath_{};
    std::string libpath_{};
    std::string libc23path_{};
    std::string includepath_{};
    std::string extrainclude_{};

    bool link2gk_{};
    std::string inputname_{};
    std::string outputname_{};

    bool summaryflag_{};
    std::string passtream_{};
    std::vector<int> modpassprint_{};
    std::vector<int> funcpassprint_{};

    TransUnit transunit_{};
    std::unique_ptr<Module> module_{};
};

#endif // _DRIVER_H_
