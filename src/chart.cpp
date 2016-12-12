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
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "chart.h"
#include "debug.h"
#include "event.h"

const std::string DEFAULT_NOTE_TRACK_EVENT_TAP = "t";
const std::string DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP = "*";

#define SONG_SECTION "Song"
#define SYNC_TRACK_SECTION "SyncTrack"
#define EVENTS_SECTION "Events"

void splitOnce(std::string& first, std::string& second, std::string str);
bool isNoteSection(const std::string& section);
std::string toFeedbackSafeString(const NoteTrackEvent& nte);

Chart::Chart() {
}

Chart::~Chart() {
}

bool Chart::read(char fpath[]) {
	bool errors = false;
	bool inBlock = false;
	std::ifstream infile(fpath);
	std::string section;

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
					if (parseNoteSectionLine(section, line))
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
	if (!extractNotesFromNoteTrackEvents())
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
bool Chart::parseNoteSectionLine(const std::string& section, const std::string& line) {
	std::string key;
	std::string value;

	// Get time and vector associated with it
	splitOnce(key, value, line, '=');
	int time = stoi(key);
	std::vector<NoteTrackEvent>& v = noteTrackEvents[section][time];

	// Parse note
	splitOnce(key, value, value, ' ');
	if (key == NOTE_TRACK_EVENT_TYPE_EVENT) { // "E" "some event"
		if (value == DEFAULT_NOTE_TRACK_EVENT_TAP) {
			// Tap event
			v.push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_TAP, 0));
			// TODO: Move event replacements to fix:: ?
			std::cerr << "Replacing track event E " << DEFAULT_NOTE_TRACK_EVENT_TAP;
			std::cerr << " at time " << time << " with tap flag" << "\r\n";
		} else if (value == DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP) {
			// HOPO flip event
			v.push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_HOPO_FLIP, 0));
			std::cerr << "Replacing track event E " << DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP;
			std::cerr << " at time " << time << " with HOPO flip flag" << "\r\n";
		} else {
			// Other track event
			// v.push_back(NoteEvent(time, value));
			std::cerr << "Removing unknown track event E " << value;
			std::cerr << " at time " << time << "\r\n";
		}
		return true;
	} else if (key == NOTE_TRACK_EVENT_TYPE_NOTE || key == NOTE_TRACK_EVENT_TYPE_STAR_POWER) { // "N" "5 0"
		std::string type = key;
		splitOnce(key, value, value, ' ');
		int val = stoi(key);
		int dur = stoi(value);
		v.push_back(NoteTrackEvent(time, type, val, dur));
		return true;
	}
	return false;
}

bool Chart::extractNotesFromNoteTrackEvents() {
	bool errors = false;
	// Iterate over map keys, which are note section names
	for (auto it : noteTrackEvents) {
		// Get the note map for this section
		std::string section = it.first;
		std::map<uint32_t, Note>& noteMap = noteTrackNotes[section];

		// Iterate over submap keys, which are values of `time`
		for (auto e1 : noteTrackEvents[section]) {
			// Get the NoteEvent vector for this value of `time`
			uint32_t time = e1.first;
			std::vector<NoteTrackEvent>& events = noteTrackEvents[section][time];
			// Parse note events into notes, inserting them into map `noteMap`
			Note note;
			note.time = events[0].time;
			note.duration = events[0].duration;
			note.value = 0;

			std::set<uint32_t> durationSet;

			// Build note bits
			for (NoteTrackEvent evt : events) {
				if (!evt.isNote())
					continue;
				if (!evt.isFlag())
					durationSet.insert(evt.time);
				note.value |= (1 << evt.value);
			}

			// Assert note durations are equal
			if (!durationSet.size() == 1) {
				// Durations are not equal, must be an extended sustain. Let's fix it
				std::cerr << "Unequal note durations detected at time " << note.time << "\r\n";
				// TODO
				// vector<Note> fixedNotes;
				// Fix::fix_unequal_note_durations(fixedNotes, simultaneousNoteEvents);
				// // Add notes to map
				// for (Note n: fixedNotes) {
				// 	noteMap[n.time] = n;
				// }
				noteMap[note.time] = note;
			} else {
				// Add note to map
				noteMap[note.time] = note;
			}
			// Remove parsed note from events vector
			// TODO
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
	std::vector<NoteTrackEvent> nte;
	for (auto const& e0 : noteTrackNotes) {
		std::string section = e0.first;
		std::map<uint32_t, Note>& notes = noteTrackNotes[section];

		std::cout << "[" << section << "]" << "\r\n" << "{" << "\r\n";

		// Iterate over each note stored in this note section's map
		for (auto const& e1 : notes) {
			uint32_t time = e1.first;
			Note note = notes[time];

			nte.clear();
			note.toNoteTrackEvents(nte);
			for (NoteTrackEvent& evt : nte) {
				std::cout << '\t' << evt.toEventString() << "\r\n";
			}
		}
		// Iterate over NoteEvents (excluding actual notes)
		for (const auto& e1 : noteTrackEvents[section]) {
			std::vector<NoteTrackEvent>& vec = noteTrackEvents[section][e1.first];
			for (const NoteTrackEvent& evt : vec) {
				if (evt.isNote())
					continue; // Ignore notes, they were done above
				std::cout << '\t' << evt.toEventString() << "\r\n";
			}
		}

		std::cout << "}" << "\r\n";
	}
}

std::string toFeedbackSafeString(const NoteTrackEvent& nte) {
	if (!nte.isFlag())
		return nte.toEventString();
	std::stringstream ss;
	ss << nte.time << " = E ";
	if (nte.value == NOTE_FLAG_VAL_HOPO_FLIP) {
		ss << DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP;
	} else if (nte.value == NOTE_FLAG_VAL_TAP) {
		ss << DEFAULT_NOTE_TRACK_EVENT_TAP;
	} else {
		std::cerr << "Unhandled NTE value for toFeedbackSafeString() conversion " << nte.value << "\r\n";
		ss << nte.value;
	}
	return ss.str();
}