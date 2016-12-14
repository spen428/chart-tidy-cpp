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

	// Parse note
	splitOnce(key, value, value, ' ');
	if (key == NOTE_TRACK_EVENT_TYPE_EVENT) { // "E" "some event"
		if (value == DEFAULT_NOTE_TRACK_EVENT_TAP) {
			// Tap event
			noteTrackEvents[section].push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_TAP, 0));
			// TODO: Move event replacements to fix:: ?
			std::cerr << "Replacing track event E " << DEFAULT_NOTE_TRACK_EVENT_TAP;
			std::cerr << " at time " << time << " with tap flag" << "\r\n";
		} else if (value == DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP) {
			// HOPO flip event
			noteTrackEvents[section].push_back(NoteTrackEvent(time, NOTE_FLAG_VAL_HOPO_FLIP, 0));
			std::cerr << "Replacing track event E " << DEFAULT_NOTE_TRACK_EVENT_HOPO_FLIP;
			std::cerr << " at time " << time << " with HOPO flip flag" << "\r\n";
		} else {
			// Other track event
			// noteTrackEvents[section].push_back(NoteEvent(time, value));
			std::cerr << "Removing unknown track event E " << value;
			std::cerr << " at time " << time << "\r\n";
		}
		return true;
	} else if (key == NOTE_TRACK_EVENT_TYPE_NOTE || key == NOTE_TRACK_EVENT_TYPE_STAR_POWER) { // "N" "5 0"
		std::string type = key;
		splitOnce(key, value, value, ' ');
		noteTrackEvents[section].push_back(NoteTrackEvent(time, type, stoi(key), stoi(value)));
		return true;
	} else {
		std::cerr << "Unrecognised key when parsing note section line: " << key << "\r\n";
	}
	return false;
}

bool Chart::extractNotesFromNoteTrackEvents() {
	bool errors = false;
	// Iterate over map keys, which are note section names
	for (const auto& it : noteTrackEvents) {
		// Get the note map for this section
		std::string section = it.first;
		std::vector<NoteTrackEvent> filteredNteVec; // Will replace noteTrackEvents[sectiom] after notes are removed

		// Filter actual notes out of the map: insert notes into `noteTrackNotes` and retain
		// other events in `noteTrackEvents`.
		for (const NoteTrackEvent& evt : noteTrackEvents[section]) {
			if (!evt.isNote()) {
				// Keep non-note event in `noteTrackEvents`
				filteredNteVec.push_back(evt);
			} else {
				// Parse note, setting the value bits, and insert into `noteTrackNotes`
				if (noteTrackNotes[section].find(evt.time) == noteTrackNotes[section].end()) {
					Note note;
					note.time = evt.time;
					note.duration = evt.duration;
					note.value = 0;
					note.value |= (1 << evt.value);
					noteTrackNotes[section][evt.time] = note;
				} else {
					noteTrackNotes[section][evt.time].value |= (1 << evt.value);
				}
			}
		}
		// Replace `noteTrackEvents[section]` with filtered vector
		noteTrackEvents[section] = filteredNteVec;
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

void Chart::print() {
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
	std::sort(syncTrack.begin(), syncTrack.end());
	for (const SyncTrackEvent& evt : syncTrack) {
		std::cout << '\t' << evt.toEventString() << "\r\n";
	}
	std::cout << "}" << "\r\n";

	std::cout << "[" << EVENTS_SECTION << "]" << "\r\n" << "{" << "\r\n";
	std::sort(events.begin(), events.end());
	for (const Event& evt : events) {
		std::cout << '\t' << evt.toEventString() << "\r\n";
	}
	std::cout << "}" << "\r\n";

	// Iterate over each note section
	std::vector<NoteTrackEvent> merged;
	for (const auto& itr0 : noteTrackNotes) {
		std::string section = itr0.first;
		mergeEvents(merged, noteTrackEvents[section], noteTrackNotes[section]);

		std::cout << "[" << section << "]" << "\r\n" << "{" << "\r\n";
		std::sort(merged.begin(), merged.end());
		for (const NoteTrackEvent& nte : merged) {
			std::cout << '\t' << nte.toEventString() << "\r\n";
		}
		std::cout << "}" << "\r\n";
		
		merged.clear();
	}
}

void Chart::mergeEvents(std::vector<NoteTrackEvent>& out, const std::vector<NoteTrackEvent>& nte,
		const std::map<uint32_t, Note>& notes) {
	for (const NoteTrackEvent& evt : nte) {
		out.push_back(evt);
	}
	for (auto const& itr : notes) {
		uint32_t time = itr.first;
		const Note& note = notes.at(time);
		note.toNoteTrackEvents(out);
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
		ss << nte.value; // Just keep it as it is
	}
	return ss.str();
}