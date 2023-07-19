#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "ast/Statement.h"
#include "IR/Value.h"

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

    void SetOutputType(OutputType ty) { outputype_ = ty; }
    void SetLink2Ginkgo(bool l) { link2gk_ = l; }
    void SetInputName(const std::string& n) { inputname_ = n; }
    void SetOutputName(const std::string& n) { outputname_ = n; }

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

    bool link2gk_{};
    std::string inputname_{};
    std::string outputname_{};

    TransUnit transunit_{};
    std::unique_ptr<Module> module_{};
};

#endif // _DRIVER_H_
