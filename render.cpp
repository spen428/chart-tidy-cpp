#include <iostream>

#include "render.h"

using namespace std;

void draw(string (&lines)[5], char c);
void draw(string (&lines)[5], char (&c)[5]);

void Renderer::chart_to_text(const Chart& chart)
{
	const unsigned int measures_per_line = 1;
	const unsigned int unit_time = 48;

	cout << "Name:   \t" << chart.name << endl;
	cout << "Artist: \t" << chart.artist << endl;
	cout << "Charter:\t" << chart.charter << endl;
	cout << endl;

	for (auto const& e0: chart.noteSections) {
		string section = e0.first;
		map<uint32_t, Note> notes = chart.noteSections.at(section);
		cout << section << endl << endl;

		unsigned int ctime = 0; // Current time
		string lines[5] = { "G", "R", "Y", "B", "O" };
		draw(lines, '|');
		for (auto const& e1: notes) {
			uint32_t time = e1.first;
			Note note = notes[time];

			time /= unit_time; // Normalise time
			// Fill empty space
			while (time > ctime) {
				ctime += 1;
				draw(lines, '-');
			}
			// Draw notes
			char c[5] = { '-', '-', '-', '-', '-' };
			for (unsigned int b = 0; b < 5; b++)
				if ((note.value >> b) & 1)
					c[b] = 'x';
			if ((note.value >> TAP_FLAG_VAL) & 1) {
				// Convert to taps
				for (int i = 0; i < 5; i++)
					if (c[i] == 'x')
						c[i] = 'e';
			}
			draw(lines, c);
		}
		draw(lines, '|');
		for (auto s: lines)
			cout << s << endl;
	}
}

void draw(string (&lines)[5], char c)
{
	char ca[5] = {c,c,c,c,c};
	draw(lines, ca);
}

void draw(string (&lines)[5], char (&c)[5])
{
	for (int i = 0; i < 5; i++)
		*(lines + i) += *(c + i);

	// Check if line is full
	// TODO: Account for time signature
	if (lines[0].length() == 48 + 2) {
		draw(lines, '|');
	} else if (lines[0].length() == 96 + 3) {
		draw(lines, '|');
		for (auto s: lines)
			cout << s << endl;
		cout << endl;
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
