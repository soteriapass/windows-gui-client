#include "log.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <grpc/support/log.h>

namespace logging
{
bool g_Verbose = false;
std::ofstream g_logFile;

void grpc_log_func(gpr_log_func_args* args)
{
    std::stringstream ss;
    ss << "grpc (" << args->file << ":" << args->line << ") " << args->message;
    logging::log(ss.str(), args->severity != GPR_LOG_SEVERITY_ERROR);
}

bool init(conf& conf_file)
{
    gpr_set_log_function(&grpc_log_func);

    const std::string log_file = conf_file.get_log_file();
    if(log_file.empty())
        return true;

    g_logFile.open(log_file.c_str(), std::ios::out);
    if(!g_logFile.is_open())
    {
        std::cout << "Could not open log file" << std::endl;
        return false;
    }

    return true;
}

void destroy()
{
    g_logFile.close();
}

void log(const std::string& message, bool verbose)
{
    if(verbose && !g_Verbose)
        return;

    if(g_logFile.is_open())
    {
        g_logFile << message << std::endl;
    }
    else
    {
        std::cout << message << std::endl;
    }
}

void set_verbose(bool verbose)
{
    g_Verbose = true;
}
}
