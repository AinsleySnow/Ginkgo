#include <cstring>
#include <string>
#include "main/Driver.h"


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
        else if (strcmp(argv[i], "-lgk") == 0)
            driver.SetLink2Ginkgo(true);
    }

    driver.Run();
}
