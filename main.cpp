#include <iostream>
#include "chart.h"
#include "fix.h"


int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "You must pass a file path as an argument" << std::endl;
		return 1;
	}

	Chart chart;
	chart.read(argv[1]);
	Fix::fix_all(chart);
	chart.print();
	return 0;
}
