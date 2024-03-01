#!/bin/bash

clang++ `pkg-config --cflags raftlib` main.cpp -o test.exe -l boost_system -l hiberlite -I/usr/include/hiberlite -I. -I../include `pkg-config --libs raftlib`

