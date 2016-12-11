#!/bin/bash
g++ -g -std=c++11 -o test test.cpp chart.cpp event.cpp fix.cpp
g++ -g -std=c++11 -o main main.cpp chart.cpp event.cpp fix.cpp
