#pragma once

#include <string>

#include "conf.h"

namespace logging
{
    bool init(conf& conf_file);
    void destroy();
    void log(const std::string& message, bool verbose);
    void set_verbose(bool verbose);
}
