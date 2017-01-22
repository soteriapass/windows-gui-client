#include "log.h"

#include <iostream>

namespace logging
{
bool g_Verbose = false;

void log(const std::string& message, bool verbose)
{
    if(verbose && !g_Verbose)
        return;

    std::cout << message << std::endl;
}

void set_verbose(bool verbose)
{
    g_Verbose = true;
}
}
