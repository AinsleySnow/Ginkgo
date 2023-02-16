#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include "main/Driver.h"


int main(int argc, char* argv[])
{
    std::string filename = "";
    std::string output = "";

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            filename = argv[i];
        else if (strcmp(argv[i], "-o"))
            output = argv[++i];
        else if (strcmp(argv[i], "-emit-ir"))
            ; // do nothing for now
    }

    if (filename.empty())
        return 1;

    Driver div = Driver(filename);
    if (!output.empty())
        div.SetOutputFile(output);

    div.Parse();
    div.GenerateIR();
    div.PrintIR();
}
