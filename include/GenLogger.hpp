#pragma once

#include <iostream>
#include <iomanip>
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

    #define Dprintf(Severity, ...) GenTraceLoc(Severity, ::getpid(), std::this_thread::get_id(), __FILE_NAME__, std::experimental::source_location::current(), __VA_ARGS__)

    template <typename ...Args>
    void GenTraceLoc(short int Severity, pid_t thePid, std::thread::id theTid, const char* theFileName, const std::experimental::source_location& location, Args&& ...args)
    {
        std::ostringstream stream;
        switch (Severity)
        {
            case GSL::INFO:
            stream << "\033[0;92m [INFO] ";
            break;
            case GSL::WARNING:
            stream << "\033[0;93m [WARN] ";
            break;
            case GSL::ERROR:
            stream << "\033[0;91m[ERROR] ";
            break;
            case GSL::FATAL:
            stream << "\033[0;96m[FATAL] ";
            break;
            default:
            stream << "\033[0;7m[UNKNWN] ";
            break;
        }
        stream << std::setw(30) << std::left << theFileName << " |Pid:"  << std::setw(7) << std::left << thePid 
               << " |ThreadId:" << std::setw(16) << std::left << theTid << " |@" 
               << std::setw(30) << std::left << location.function_name() << " |line#"  << std::setw(6) << std::left << location.line() << "| ";

        (stream << ... << std::forward<Args>(args)) << "\033[0m" << std::endl;

        std::cout << stream.str();
    }
}