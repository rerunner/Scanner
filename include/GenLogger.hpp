#pragma once

#include <thread>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <experimental/source_location>

namespace GSL {
    enum MessageLevel 
    { 
        INFO     = 0x00000001, 
        DEBUG    = 0x00000010, 
        WARNING  = 0x00000100, 
        ERROR    = 0x00001000,
        FATAL    = 0x00010000
    };

    extern unsigned int ACTIVE_MESSAGES;

    #define Dprintf(Severity, ...) GenTraceLoc(Severity, ::getpid(), std::this_thread::get_id(), __FILE_NAME__, std::experimental::source_location::current(), __VA_ARGS__)

    template <typename ...Args>
    void GenTraceLoc(unsigned int Severity, pid_t thePid, std::thread::id theTid, const char* theFileName, const std::experimental::source_location& location, Args&& ...args)
    {
        std::ostringstream stream;
        if (Severity & ACTIVE_MESSAGES)
        {
            switch (Severity)
            {
                case GSL::INFO:
                stream << "[INFO]  ";
                break;
                case GSL::DEBUG:
                stream << "\033[0;92m[DEBUG] ";
                break;
                case GSL::WARNING:
                stream << "\033[0;93m[WARN]  ";
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
}