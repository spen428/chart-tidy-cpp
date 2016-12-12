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
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "chart.h"
#include "debug.h"
#include "event.h"

#define TRACK_EVENT_TAP "t"
#define TRACK_EVENT_HOPO_FLIP "*"

#define SONG_SECTION "Song"
#define SYNC_TRACK_SECTION "SyncTrack"
#define EVENTS_SECTION "Events"

void splitOnce(std::string& first, std::string& second, std::string str);
bool isNoteSection(const std::string& section);

Chart::Chart() {
}

Chart::~Chart() {
}

bool Chart::read(char fpath[]) {
	bool errors = false;
	bool inBlock = false;
	std::ifstream infile(fpath);
	std::string section;
	std::unordered_map<std::string, std::map<uint32_t, std::vector < NoteTrackEvent>>> mNoteEvents;

	for (std::string line; getline(infile, line);) {
		boost::trim(line);

		if (line == "")
			continue; // Skip blank lines

		if (!inBlock) {
			if (section == "" && line.at(0) == '[') {
				// Begin section header
				if (line.at(line.length() - 1) == ']') {
					// Section header is on its own line, usual case
					section = line.substr(1, line.length() - 2);
					DEBUG("SECTION HEADER: " + section);
				} else {
					std::cerr << "Unhandled syntax: " << line << "\r\n";
					errors = true;
				}
			} else if (line == "{") {
				// Start of section block
				inBlock = true;
				DEBUG("BEGIN SECTION");
			} else {
				std::cerr << "Illegal state for line: " << line << "\r\n";
				errors = true;
			}
			continue;
		} else if (line == "}") {
			// End of section block
			inBlock = false;
			section = "";
			DEBUG("END SECTION");
			continue;
		} else {
			if (section == SONG_SECTION) {
				if (parseSongLine(line))
					continue;
			} else if (section == SYNC_TRACK_SECTION) {
				if (parseSyncTrackLine(line))
					continue;
			} else if (section == EVENTS_SECTION) {
				if (parseEventsLine(line))
					continue;
			} else {
				if (isNoteSection(section)) {
					if (parseNoteSectionLine(mNoteEvents[section], line))
						continue;
				} else {
					std::cerr << "Unknown section: " << section << "\r\n";
					errors = true;
				}
			}
		}
		std::cerr << "Unexpected line: " << line << "\r\n";
		errors = true;
	}
	infile.close();
	if (!parseNoteEvents(mNoteEvents))
		errors = true;
	return !errors;
}

/**
 * Split a string `str` into two substrings `first` and `second` and trim the
 * whitespace from them. If `delim` is not found in `str`, `first` will equal
 * `str` after trimming and `second` will equal "".
 */
void splitOnce(std::string& first, std::string& second, const std::string& str, char delim) {
	const auto idx = str.find_first_of(delim);
	if (std::string::npos != idx) {
		first = str.substr(0, idx);
		second = str.substr(idx + 1);
	} else {
		first = str.substr(0);
		second = "";
	}
	boost::trim(first);
	boost::trim(second);
}

bool Chart::parseSongLine(const std::string& line) {
	std::string key;
	std::string value;
	splitOnce(key, value, line, '=');
	if (key == "Name") {
		name = value;
	} else if (key == "Artist") {
		artist = value;
	} else if (key == "Charter") {
		charter = value;
	} else if (key == "Offset") {
		offset = stod(value);
	} else if (key == "Resolution") {
		resolution = stoi(value);
	} else if (key == "Player2") {
		player2 = value;
	} else if (key == "Difficulty") {
		difficulty = stoi(value);
	} else if (key == "PreviewStart") {
		previewStart = stod(value);
	} else if (key == "PreviewEnd") {
		previewEnd = stod(value);
	} else if (key == "Genre") {
		genre = value;
	} else if (key == "MediaType") {
		mediaType = value;
	} else if (key == "MusicStream") {
		musicStream = value;
	} else {
		std::cerr << "Unknown key: " << key << "\r\n";
		return false;
	}
	return true;
}

bool Chart::parseSyncTrackLine(const std::string& line) {
	std::string key;
	std::string value;

	// Get time
	splitOnce(key, value, line, '=');
	int time = stoi(key);

	// Get event details
	splitOnce(key, value, value, ' ');
	int val = stoi(value);
	syncTrack.push_back(SyncTrackEvent(time, key, val));
	return true;
}

bool Chart::parseEventsLine(const std::string& line) {
	std::string key;
	std::string value;

	// Get time
	splitOnce(key, value, line, '=');
	int time = stoi(key);

	// Get event details
	splitOnce(key, value, value, ' ');
	if (key == "E") {
		events.push_back(Event(time, value));
		return true;
	}
	return false;
}

/**
 * Parse a line into a NoteEvent object and insert it into the vector
 * `noteEvents`.
 */
bool Chart::parseNoteSectionLine(std::map<uint32_t, std::vector<NoteTrackEvent>>&noteEvents,
		const std::string& line) {
	std::string key;
	std::string value;

	// Get time and vector associated with it
	splitOnce(key, value, line, '=');
	int time = stoi(key);
	std::vector<NoteTrackEvent>& v = noteEvents[time];

	// Parse note
	splitOnce(key, value, value, ' ');
	if (key == "E") { // "E" "some event"
		if (value == TRACK_EVENT_TAP) {
			// Tap event
			v.push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_TAP, 0));
			std::cerr << "Replacing track event E " << TRACK_EVENT_TAP;
			std::cerr << " at time " << time << " with tap flag" << "\r\n";
		} else if (value == TRACK_EVENT_HOPO_FLIP) {
			// HOPO flip event
			v.push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_HOPO_FLIP, 0));
			std::cerr << "Replacing track event E " << TRACK_EVENT_HOPO_FLIP;
			std::cerr << " at time " << time << " with HOPO flip flag" << "\r\n";
		} else {
			// Other track event
			// v.push_back(NoteEvent(time, value));
			std::cerr << "Removing unknown track event E " << value;
			std::cerr << " at time " << time << "\r\n";
		}
		return true;
	} else if (key == "N") { // "N" "5 0"
		// Note
		splitOnce(key, value, value, ' ');
		int val = stoi(key);
		int dur = stoi(value);
		v.push_back(NoteTrackEvent(time, val, dur));
		return true;
	}
	return false;
}

bool Chart::parseNoteEvents(std::unordered_map<std::string, std::map<uint32_t, std::vector<NoteTrackEvent>>>& noteEvents) {
	bool errors = false;
	// Iterate over map keys, which are note section names
	for (auto it : noteEvents) {
		// Get the note map for this section
		std::string section = it.first;
		std::map<uint32_t, Note>& noteTrack = noteTracks[section];

		// Iterate over submap keys, which are values of `time`
		for (auto e1 : noteEvents[section]) {
			// Get the NoteEvent vector for this value of `time`
			uint32_t time = e1.first;
			std::vector<NoteTrackEvent> events = noteEvents[section][time];
			// Parse note events into notes, inserting them into map `m`
			Note::parseNotes(noteTrack, events);
		}
	}
	return !errors;
}

bool isNoteSection(const std::string& section) {
	return ((boost::starts_with(section, "Easy")
			|| boost::starts_with(section, "Medium")
			|| boost::starts_with(section, "Hard")
			|| boost::starts_with(section, "Expert"))
			&&
			(boost::ends_with(section, "Single")
			|| boost::ends_with(section, "DoubleGuitar")
			|| boost::ends_with(section, "DoubleBass")
			|| boost::ends_with(section, "EnhancedGuitar")
			|| boost::ends_with(section, "CoopLead")
			|| boost::ends_with(section, "CoopBass")
			|| boost::ends_with(section, "10KeyGuitar")
			|| boost::ends_with(section, "Drums")
			|| boost::ends_with(section, "DoubleDrums")
			|| boost::ends_with(section, "Vocals")
			|| boost::ends_with(section, "Keyboard")));
}

#define DEFAULT_FEEDBACK_SAFE false

void Chart::print() {
	print(DEFAULT_FEEDBACK_SAFE);
}

void Chart::print(bool feedbackSafe) {
	std::cout << "[" << SONG_SECTION << "]" << "\r\n" << "{" << "\r\n";
	std::cout << '\t' << "Name" << " = " << name << "\r\n";
	std::cout << '\t' << "Artist" << " = " << artist << "\r\n";
	std::cout << '\t' << "Charter" << " = " << charter << "\r\n";
	std::cout << '\t' << "Offset" << " = " << offset << "\r\n";
	std::cout << '\t' << "Resolution" << " = " << resolution << "\r\n";
	std::cout << '\t' << "Player2" << " = " << player2 << "\r\n";
	std::cout << '\t' << "Difficulty" << " = " << difficulty << "\r\n";
	// TODO: How many d.p. are expected?
	std::cout << '\t' << "PreviewStart" << " = " << previewStart << "\r\n";
	std::cout << '\t' << "PreviewEnd" << " = " << previewEnd << "\r\n";
	std::cout << '\t' << "Genre" << " = " << genre << "\r\n";
	std::cout << '\t' << "MediaType" << " = " << mediaType << "\r\n";
	std::cout << '\t' << "MusicStream" << " = " << musicStream << "\r\n";
	std::cout << "}" << "\r\n";

	std::cout << "[" << SYNC_TRACK_SECTION << "]" << "\r\n" << "{" << "\r\n";
	for (const SyncTrackEvent& evt : syncTrack) {
		std::cout << '\t' << evt.toEventString() << "\r\n";
	}
	std::cout << "}" << "\r\n";

	std::cout << "[" << EVENTS_SECTION << "]" << "\r\n" << "{" << "\r\n";
	for (const Event& evt : events) {
		std::cout << '\t' << evt.toEventString() << "\r\n";
	}
	std::cout << "}" << "\r\n";

	// Iterate over each note section
	for (auto const& e0 : noteTracks) {
		std::string section = e0.first;
		std::map<uint32_t, Note>& notes = noteTracks[section];

		std::cout << "[" << section << "]" << "\r\n" << "{" << "\r\n";

		// Iterate over each note stored in this note section's map
		for (auto const& e1 : notes) {
			uint32_t time = e1.first;
			Note note = notes[time];

			// Write each of the active note flags out
			for (unsigned int b = 0; b < 32; b++) {
				if (!((note.value >> b) & 1))
					continue;
				std::cout << '\t' << note.time << " = ";
				if (b > PLAYABLE_NOTE_TOTAL) {
					if (feedbackSafe) {
						if (b == NOTE_FLAG_VAL_HOPO_FLIP) {
							std::cout << "E " << TRACK_EVENT_HOPO_FLIP << "\r\n";
						} else if (b == NOTE_FLAG_VAL_TAP) {
							std::cout << "E " << TRACK_EVENT_TAP << "\r\n";
						} else {
							std::cerr << "Unhandled note flag " << b << "\r\n";
						}
					} else {
						std::cout << "N " << b << " ";
						// Non-playble note flags should have a duration of zero
						std::cout << 0 << "\r\n";
					}
				} else {
					std::cout << "N" << " " << b << " ";
					std::cout << note.duration << "\r\n";
				}
			}
		}

		std::cout << "}" << "\r\n";
	}
}
