#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "IR/Value.h"
#include "parser/yacc.hh"

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
    Driver(OutputType ty, const char* e, const std::string& in);
    Driver(OutputType ty, const char* e, const std::string& in, const std::string& out) :
        outputype_(ty), environment_(e), inputname_(in), outputname_(out), afterpp_(in) {}

    void Compile();
    void EmitBinary();
    void EmitAssembly();
    void EmitIntermediate();

private:
    void Preprocess();
    void Parse();
    const auto& GetAST();
    bool CheckAST();
    void GenerateIR();

    OutputType outputype_{};
    const char* environment_{};

    std::string inputname_{};
    std::string outputname_{};
    std::string afterpp_{};

    TransUnit transunit_{};
    std::unique_ptr<Module> module_{};
};

#endif // _DRIVER_H_
