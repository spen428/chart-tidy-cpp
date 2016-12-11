#!/bin/bash
./compile.sh && ./a.out test2.chart 1> out.txt
if [[ "$(diff ltest.chart out.txt)" != "" ]]; then
	diff test2fixed.chart out.txt
	echo "Test failed."
fi
