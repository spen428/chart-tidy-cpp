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
#include "cmdline.h"

#include "FeedBack.h"
#include "chart.h"
#include "debug.h"
#include "fix.h"

const std::string DEFAULT_NOTE_TRACK_EVENT_TAP = "t";
const std::string DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP = "*";

int main(int argc, char* argv[]) {
	cmdline::parser parser;
	parser.add("help", '?', "Print command line help");
	parser.add<std::string>("tap-event", 't', "The track event text that marks a tap note",
			false, DEFAULT_NOTE_TRACK_EVENT_TAP);
	parser.add<std::string>("hopo-event", 'h', "The track event text that marks a HOPO flip"
			" (force note)", false, DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP);
	parser.add("feedback-safe", 'b', "Ensure that note flags remain as (or are converted to)"
			" track events to ensure that the chart can still be safely edited in FeedBack");
	parser.add("fix-all", 'a', "Automatically apply all chart error fixes");
	parser.add<unsigned int>("sustain-gap", 'g', "The minimum gap to enforce after the end"
			" of a sustain note. Default: 24 (1/32)", false, DURATION_1_32);
	parser.add("stdio", 's', "Read in from stdin and output to stdout");

	parser.parse_check(argc, argv);
	
	if (parser.exist("help")) {
		parser.usage();
		return 0;
	}
	
	std::vector<std::string> input_files;
	if (parser.rest().size() == 0) { // Positional arguments
		if (!parser.exist("stdio")) {
			std::cerr << "You must provide at least one input file unless specifying"
					" the --stdio option. See --help\r\n";
			return 1;
		} else {
			input_files.push_back("-"); // Add stdin
		}
	} else {
		for (std::string s : parser.rest())
			input_files.push_back(s);
	}

	for (std::string input_file : input_files) {
		// Parse
		Chart chart;
		chart.track_event_hopo_flip = parser.get <std::string>("hopo-event");
		chart.track_event_tap = parser.get <std::string>("tap-event");
		chart.min_sustain_gap = parser.get<unsigned int>("sustain-gap");
		chart.read(input_file);

		// Apply fixes
		if (parser.exist("fix-all"))
			fix::fixAll(chart);
		if (parser.exist("feedback-safe"))
			fix::unsetNoteFlags(chart);
		else
			fix::setNoteFlags(chart);

		// Output
		if (parser.exist("stdio")) {
			chart.write("-");
		} else {
			// Prepend "fixed_" to input file name to get output file name
			#ifdef _WIN32
			const char sep = '\\';
			#else
			const char sep = '/';
			#endif
			std::string output_file = input_file;
			size_t idx = input_file.rfind(sep, input_file.length());
			if (idx != std::string::npos)
				output_file = input_file.substr(idx + 1, input_file.length() - idx);

			// Write to disk
			chart.write("fixed_" + output_file);
		}
	}
	return 0;
}
