#!/bin/bash
g++ -g -std=c++11 test.cpp chart.cpp event.cpp && ./a.out test2.chart 1> out.txt
if [[ "$(diff ltest.chart out.txt)" != "" ]]; then
	diff test2fixed.chart out.txt
	echo "Test failed."
fi
