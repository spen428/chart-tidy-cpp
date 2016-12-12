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

#include "render.h"

void draw(std::string(&lines)[5], char c);
void draw(std::string(&lines)[5], char (&c)[5]);

void renderer::chartToText(const Chart& chart) {
	const unsigned int measures_per_line = 1;
	const unsigned int unit_time = 48;

	std::cout << "Name:   \t" << chart.name << "\r\n";
	std::cout << "Artist: \t" << chart.artist << "\r\n";
	std::cout << "Charter:\t" << chart.charter << "\r\n";
	std::cout << "\r\n";

	for (auto const& e0 : chart.noteTrackNotes) {
		std::string section = e0.first;
		std::map<uint32_t, Note> notes = chart.noteTrackNotes.at(section);
		std::cout << section << "\r\n" << "\r\n";

		unsigned int ctime = 0; // Current time
		std::string lines[5] = {"G", "R", "Y", "B", "O"};
		draw(lines, '|');
		for (auto const& e1 : notes) {
			uint32_t time = e1.first;
			Note note = notes[time];

			time /= unit_time; // Normalise time
			// Fill empty space
			while (time > ctime) {
				ctime += 1;
				draw(lines, '-');
			}
			// Draw notes
			char c[5] = {'-', '-', '-', '-', '-'};
			for (unsigned int b = 0; b < 5; b++)
				if ((note.value >> b) & 1)
					c[b] = 'x';
			if ((note.value >> NOTE_FLAG_VAL_TAP) & 1) {
				// Convert to taps
				for (int i = 0; i < 5; i++)
					if (c[i] == 'x')
						c[i] = 'e';
			}
			draw(lines, c);
		}
		draw(lines, '|');
		for (auto s : lines)
			std::cout << s << "\r\n";
	}
}

void draw(std::string(&lines)[5], char c) {
	char ca[5] = {c, c, c, c, c};
	draw(lines, ca);
}

void draw(std::string(&lines)[5], char (&c)[5]) {
	for (int i = 0; i < 5; i++)
		*(lines + i) += *(c + i);

	// Check if line is full
	// TODO: Account for time signature
	if (lines[0].length() == 48 + 2) {
		draw(lines, '|');
	} else if (lines[0].length() == 96 + 3) {
		draw(lines, '|');
		for (auto s : lines)
			std::cout << s << "\r\n";
		std::cout << "\r\n";
		for (int i = 0; i < 5; i++)
			lines[i].clear();
		lines[0] += 'G';
		lines[1] += 'R';
		lines[2] += 'Y';
		lines[3] += 'B';
		lines[4] += 'O';
		draw(lines, '|');
	}
}
