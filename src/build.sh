#!/bin/bash

clang++ `pkg-config --cflags raftlib` main.cpp ./MachineControl/application/MachineControl.cpp -o test.exe -l boost_system -l hiberlite -I/usr/include/hiberlite -I. -I./Measure/Leveling -I./MachineControl -I../include `pkg-config --libs raftlib`

