#include <cstring>
#include <string>
#include "main/Driver.h"


std::pair<int, bool> PassName2Index(const char* name)
{
    if (strcmp(name, "FlowGraph") == 0)
        return std::make_pair(100, true);
    else if (strcmp(name, "DUInfo") == 0)
        return std::make_pair(200, true);
    else if (strcmp(name, "LoopAnalyze") == 0)
        return std::make_pair(300, true);
    else if (strcmp(name, "Liveness") == 0)
        return std::make_pair(400, true);
    else if (strcmp(name, "SimpleAlloc") == 0)
        return std::make_pair(500, true);
    return std::make_pair(0, false);
}


int main(int argc, char* argv[])
{
    OutputType outputype = OutputType::binary;
    Driver driver{ argv[0] };

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            driver.SetInputName(argv[i]);
        else if (strcmp(argv[i], "-o") == 0)
            driver.SetOutputName(argv[++i]);
        else if (strcmp(argv[i], "-emit-ir") == 0)
            driver.SetOutputType(OutputType::intermediate);
        else if (strcmp(argv[i], "-S") == 0)
            driver.SetOutputType(OutputType::assembly);
        else if (strcmp(argv[i], "-I") == 0)
            driver.AddIncludeDir(argv[++i]);
        else if (strcmp(argv[i], "-lgk") == 0)
            driver.SetLink2Ginkgo(true);
        else if (strcmp(argv[i], "-pass-summary") == 0)
        {
            driver.SetSummaryFlag();
            i += 1;
            while (i < argc)
            {
                if (argv[i][0] == '-')
                {
                    i -= 1;
                    break;
                }
                else if (strcmp(argv[i], "to") == 0)
                {
                    driver.SetSummaryStream(argv[++i]);
                    i += 1;
                }
                else
                {
                    auto [index, isfunc] = PassName2Index(argv[i++]);
                    if (isfunc)
                        driver.AddFuncPass2Print(index);
                    else
                        driver.AddModulePass2Print(index);
                }
            }
        }
    }

    driver.Run();
}
