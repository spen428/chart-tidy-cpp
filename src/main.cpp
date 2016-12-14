/**
 *  chart-tidy - A tool for automatically fixing Guitar Hero III song charts.
 *
 *  Copyright (C) 2016  lykat1
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <string.h>
#include "chart.h"
#include "debug.h"
#include "fix.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "You must pass a file path as an argument" << "\r\n";
		return 1;
	}

	bool feedbackSafe = false;
	if (argc >= 3 && strcmp(argv[2], "-fb") == 0) {
		// Whether to output FeedBack-safe notes (no HOPO/tap flag)
		feedbackSafe = true;
	}

	Chart chart;
	chart.read(argv[1]);

	fix::fixAll(chart);
	if (feedbackSafe) {
		fix::unsetNoteFlags(chart);
	} else {
		fix::setNoteFlags(chart);
	}

	chart.print();
	return 0;
}
