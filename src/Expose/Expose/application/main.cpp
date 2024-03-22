// main.cpp : This file contains the 'main' function of the expose service
//

#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <variant>        // std::visit

#include "Expose/Expose/application/Expose.hpp"

int main()
{
    Expose::Expose expose;

    return 0;
}
//std::this_thread::sleep_for(std::chrono::milliseconds(1000));