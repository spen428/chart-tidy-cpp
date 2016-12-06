#include <iostream>
#include "chart.h"


int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "You must pass a file path as an argument" << std::endl;
		return 1;
	}

	Chart chart;
	chart.read(argv[1]);
	chart.print();
	return 0;
}
