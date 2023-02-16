#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "IR/Value.h"
#include "parser/yacc.hh"


class Driver
{
public:
    Driver() {}
    Driver(const std::string& in) : inputname_(in) {}
    Driver(const std::string& in, const std::string& out) :
        inputname_(in), outputname_(out) {}
    ~Driver() {}

    void Parse();
    const auto& GetAST();
    bool CheckAST();
    void GenerateIR();
    void PrintIR();

    void SetInputFile(const std::string& in) { inputname_ = in; }
    auto GetInputFile() { return inputname_; }
    void SetOutputFile(const std::string& out) { outputname_ = out; }
    auto GetOutputFile() { return outputname_; }

private:
    std::string inputname_{};
    std::string outputname_{};

    TransUnit transunit_{};
    std::unique_ptr<Module> module_{};
};

#endif // _DRIVER_H_
