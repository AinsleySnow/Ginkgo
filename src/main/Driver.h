#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "IR/Value.h"
#include "parser/yacc.hh"


class Driver
{
public:
    Driver(const std::string&);
    Driver(const std::string&, const std::string&);
    ~Driver();

    void Parse();
    const auto& GetAST();
    void CheckAST();
    void GenerateIR();


private:
    std::string inputname_{};
    std::string outputname_{};
    std::ifstream input_{};
    std::ofstream output_{};

    std::vector<std::unique_ptr<DeclStmt>> transunits_{};
    std::unique_ptr<Module> module_{};
};

#endif // _DRIVER_H_
