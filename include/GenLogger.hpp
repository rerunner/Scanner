#pragma once

#include <thread>
#include <chrono> 
#include <ctime> 
#include <ios>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <experimental/source_location>

namespace GSL {
    struct time_of_day_clock {};
    using time_of_day = std::chrono::time_point<time_of_day_clock, std::chrono::seconds>;

    enum MessageLevel 
    { 
        INFO     = 0x00000001, 
        DEBUG    = 0x00000010, 
        WARNING  = 0x00000100, 
        ERROR    = 0x00001000,
        FATAL    = 0x00010000,
        VERBOSE  = 0x00100000
    };

    extern unsigned int ACTIVE_MESSAGES;

    // Format to HH:MM:SS

#if defined __clang__
    #define Dprintf(Severity, ...) GenTraceLoc(Severity, ::getpid(), std::this_thread::get_id(), __FILE_NAME__, std::experimental::source_location::current(), __VA_ARGS__)
#else
    #define Dprintf(Severity, ...) GenTraceLoc(Severity, ::getpid(), std::this_thread::get_id(), __FILE__, std::experimental::source_location::current(), __VA_ARGS__)
#endif

    template <typename ...Args>
    void GenTraceLoc(unsigned int Severity, pid_t thePid, std::thread::id theTid, const char* theFileName, const std::experimental::source_location& location, Args&& ...args)
    {
        std::ostringstream stream;
        if (Severity & ACTIVE_MESSAGES)
        {
            
            switch (Severity)
            {
                case GSL::INFO:
                stream << " I ";
                break;
                case GSL::DEBUG:
                stream << "\033[0;92m D ";
                break;
                case GSL::WARNING:
                stream << "\033[0;93m W ";
                break;
                case GSL::ERROR:
                stream << "\033[0;91m E ";
                break;
                case GSL::FATAL:
                stream << "\033[0;96m F ";
                break;
                default:
                stream << "\033[0;7m U ";
                break;
            }

            if (GSL::VERBOSE & ACTIVE_MESSAGES)
            {
                {
                    using namespace std::chrono;
                    // get current time
                    auto now = system_clock::now();
                    // get number of milliseconds for the current second
                    // (remainder after division into seconds)
                    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
                    // convert to std::time_t in order to convert to std::tm (broken time)
                    auto timer = system_clock::to_time_t(now);
                    // convert to broken time
                    std::tm bt = *std::localtime(&timer);
                    // Print
                    stream << "|" << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
                    stream << '.' << std::setw(3) << ms.count();
                    stream << "|";
                }
            
                stream << std::setw(25) << std::left << theFileName << " |Pid:"  << std::setw(7) << std::left << thePid 
                    << " |Tid:" << std::setw(16) << std::left << theTid << " |@" 
                    << std::setw(25) << std::left << location.function_name() << " |line#"  << std::setw(6) << std::left << location.line() << "| ";
            }
            (stream << ... << std::forward<Args>(args)) << "\033[0m" << std::endl;

            std::cout << stream.str();
        }
    }
}