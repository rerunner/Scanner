#pragma once

#include <iostream>
#include <string_view>
#include <experimental/source_location>

namespace GSL {
    enum MessageLevel 
    { 
        INFO, 
        WARNING, 
        ERROR,
        FATAL
    };

    #define Dprintf(Severity, ...) GenTraceLoc(Severity, __FILE_NAME__, std::experimental::source_location::current(), __VA_ARGS__)

    template <typename ...Args>
    void GenTraceLoc(short int Severity, const char* theFileName, const std::experimental::source_location& location, Args&& ...args)
    {
        std::ostringstream stream;
        switch (Severity)
        {
            case GSL::INFO:
            stream << "\033[0;32m[INFO] ";
            break;
            case GSL::WARNING:
            stream << "\033[0;33m[WARNING] ";
            break;
            case GSL::ERROR:
            stream << "\033[0;31m[ERROR] ";
            break;
            case GSL::FATAL:
            stream << "\033[0;36m[FATAL] ";
            break;
            default:
            stream << "\033[0;7m[UNKNOWN] ";
            break;
        }
        stream << theFileName << "|" << location.function_name() << "|" << location.line() << "| ";
        (stream << ... << std::forward<Args>(args)) << "\033[0m" << std::endl;

        std::cout << stream.str();
    }
}