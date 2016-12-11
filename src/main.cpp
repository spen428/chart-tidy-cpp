#include <iostream>
#include <string.h>
#include "chart.h"
#include "fix.h"


int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "You must pass a file path as an argument" << std::endl;
		return 1;
	}

	bool feedback_safe = false;
	if (argc >= 3 && strcmp(argv[2], "-fb") == 0) {
		// Whether to output FeedBack-safe notes (no HOPO/tap flag)
		feedback_safe = true;
	}
	
	Chart chart;
	chart.read(argv[1]);
	fix::fix_all(chart);
	chart.print(feedback_safe);
	return 0;
}
