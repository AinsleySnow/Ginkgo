#include <cstring>
#include <string>
#include "main/Driver.h"


int main(int argc, char* argv[])
{
    std::string filename = "";
    std::string outputname = "";
    OutputType outputype = OutputType::binary;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            filename = argv[i];
        else if (strcmp(argv[i], "-o"))
            outputname = argv[++i];
        else if (strcmp(argv[i], "-emit-ir"))
            outputype = OutputType::intermediate;
        else if (strcmp(argv[i], "-S"))
            outputype = OutputType::assembly;
    }

    if (filename.empty())
        return 1;

    Driver div = Driver(outputype, outputname);
    div.Compile();

    if (outputype == OutputType::binary)
        div.EmitBinary();
    else if (outputype == OutputType::assembly)
        div.EmitAssembly();
    else if (outputype == OutputType::intermediate)
        div.EmitIntermediate();
}
