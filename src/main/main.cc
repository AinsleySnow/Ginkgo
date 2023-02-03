#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include "main/Driver.h"


int main(int argc, char* argv[])
{
    std::string filename = "";
    std::string output = "";

    for (int i = 0; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            filename = argv[i];
        else if (strcmp(argv[i], "-o"))
            output = argv[++i];
    }

    Driver div = Driver(filename);
    div.Parse();
}
