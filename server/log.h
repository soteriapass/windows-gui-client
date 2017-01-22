#pragma once

#include <string>

namespace logging
{
    void log(const std::string& message, bool verbose);
    void set_verbose(bool verbose);
}
