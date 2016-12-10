#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "chart.h"
#include "event.h"

#define TRACK_EVENT_TAP "t"
#define TRACK_EVENT_HOPO_FLIP "*"

#define SONG_SECTION "Song"
#define SYNC_TRACK_SECTION "SyncTrack"
#define EVENTS_SECTION "Events"

using namespace std;

void split_once(string& first, string& second, string str);
bool is_note_section(const string& section);

Chart::Chart(){}
Chart::~Chart(){}

bool Chart::read(char fpath[])
{
	bool errors = false;
	bool in_block = false;
	ifstream infile(fpath);
	string section = "";
	unordered_map<string, map<uint32_t, vector<NoteEvent>>> mNoteEvents;

	for (string line; getline(infile, line);) {
		boost::trim(line);

		if (line == "")
			continue; // Skip blank lines

		if (!in_block) {
			if (section == "" && line.at(0) == '[') {
				// Begin section header
				if (line.at(line.length() - 1) == ']') {
					// Section header is on its own line, usual case
					section = line.substr(1, line.length() - 2);
					cerr << "SECTION HEADER: " << section << endl;
				} else {
					cerr << "Unhandled syntax: " << line << endl;
					errors = true;
				}
			} else if (line == "{") {
				// Start of section block
				in_block = true;
				cerr << "BEGIN SECTION" << endl;
			} else {
				cerr << "Illegal state for line: " << line << endl;
				errors = true;
			}
			continue;
		} else if (line == "}") {
			// End of section block
			in_block = false;
			section = "";
			cerr << "END SECTION" << endl;
			continue;
		} else {
			if (section == SONG_SECTION) {
				if (parse_song_line(line))
					continue;
			} else if (section == SYNC_TRACK_SECTION) {
				if (parse_sync_track_line(line))
					continue;
			} else if (section == EVENTS_SECTION) {
				if (parse_events_line(line))
					continue;
			} else {
				if (is_note_section(section)) {
					if (parse_note_section_line(mNoteEvents[section], line))
						continue;
				} else {
					cerr << "Unknown section: " << section << endl;
					errors = true;
				}
			}
		}
		cerr << "Unexpected line: " << line << endl;
		errors = true;
	}
	infile.close();
	if (!parse_note_events(mNoteEvents))
		errors = true;
	return !errors;
}

/**
 * Split a string `str` into two substrings `first` and `second` and trim the
 * whitespace from them. If `delim` is not found in `str`, `first` will equal
 * `str` after trimming and `second` will equal "".
 */
void split_once(string& first, string& second, const string& str, char delim)
{
	const auto idx = str.find_first_of(delim);
	if (string::npos != idx) {
		first = str.substr(0, idx);
		second = str.substr(idx + 1);
	} else {
		first = str.substr(0);
		second = "";
	}
	boost::trim(first);
	boost::trim(second);
}

bool Chart::parse_song_line(const string& line)
{
	string key;
	string value;
	split_once(key, value, line, '=');
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
		cerr << "Unknown key: " << key << endl;
		return false;
	}
	return true;
}

bool Chart::parse_sync_track_line(const string& line)
{
	string key;
	string value;

	// Get time
	split_once(key, value, line, '=');
	int time = stoi(key);

	// Get event details
	split_once(key, value, value, ' ');
	int val = stoi(value);
	syncTrack.push_back(SyncTrackEvent(time, key, val));
	return true;
}

bool Chart::parse_events_line(const string& line)
{
	string key;
	string value;

	// Get time
	split_once(key, value, line, '=');
	int time = stoi(key);

	// Get event details
	split_once(key, value, value, ' ');
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
bool Chart::parse_note_section_line(map<uint32_t, vector<NoteEvent>>& noteEvents,
		const string& line)
{
	string key;
	string value;

	// Get time and vector associated with it
	split_once(key, value, line, '=');
	int time = stoi(key);
	vector<NoteEvent>& v = noteEvents[time];

	// Parse note
	split_once(key, value, value, ' ');
	if (key == "E") { // "E" "some event"
		if (value == TRACK_EVENT_TAP) {
			// Tap event
			v.push_back(NoteEvent(time, TAP_FLAG_VAL, 0));
			cerr << "Replacing track event E " << TRACK_EVENT_TAP;
			cerr << " at time " << time << " with tap flag" << endl;
		} else if (value == TRACK_EVENT_HOPO_FLIP) {
			// HOPO flip event
			v.push_back(NoteEvent(time, HOPO_FLIP_FLAG_VAL, 0));
			cerr << "Replacing track event E " << TRACK_EVENT_HOPO_FLIP;
			cerr << " at time " << time << " with HOPO flip flag" << endl;
		} else {
			// Other track event
			// v.push_back(NoteEvent(time, value));
			cerr << "Removing unknown track event E " << value;
			cerr << " at time " << time << endl;
		}
		return true;
	} else if (key == "N") { // "N" "5 0"
		// Note
		split_once(key, value, value, ' ');
		int val = stoi(key);
		int dur = stoi(value);
		v.push_back(NoteEvent(time, val, dur));
		return true;
	}
	return false;
}

bool Chart::parse_note_events(unordered_map<string, map<uint32_t, vector<NoteEvent>>>& noteEvents)
{
	bool errors = false;
	// Iterate over map keys, which are note section names
	for (auto e0: noteEvents) {
		// Get the note map for this section
		string section = e0.first;
		map<uint32_t, Note>& m = noteSections[section];

		// Iterate over submap keys, which are values of `time`
		for (auto e1: noteEvents[section]) {
			// Get the NoteEvent vector for this value of `time`
			uint32_t time = e1.first;
			vector<NoteEvent> events = noteEvents[section][time];
			// Parse note events into notes, inserting them into map `m`
			Note::parse_notes(m, events);
		}
	}
	return !errors;
}

bool is_note_section(const string& section)
{
	return (section == "EasySingle"
	        || section == "MediumSingle"
	        || section == "HardSingle"
	        || section == "ExpertSingle"
	        || section == "HardDoubleGuitar"
	        || section == "HardDoubleBass"
	        || section == "HardEnhancedGuitar"
	        || section == "HardCoopLead"
	        || section == "HardCoopBass"
	        || section == "Hard10KeyGuitar"
	        || section == "HardDrums"
	        || section == "HardDoubleDrums"
	        || section == "HardVocals"
	        || section == "HardKeyboard");
}

#define BEGIN_SECTION(X) cout << "[" << X << "]" << endl << "{" << endl
#define END_SECTION() cout << "}" << endl
void Chart::print()
{
	BEGIN_SECTION(SONG_SECTION);
	cout << '\t' << "Name" << " = " << name << endl;
	cout << '\t' << "Artist" << " = " << artist << endl;
	cout << '\t' << "Charter" << " = " << charter << endl;
	cout << '\t' << "Offset" << " = " << offset << endl;
	cout << '\t' << "Resolution" << " = " << resolution << endl;
	cout << '\t' << "Player2" << " = " << player2 << endl;
	cout << '\t' << "Difficulty" << " = " << difficulty << endl;
	// TODO: How many d.p. are expected?
	cout << '\t' << "PreviewStart" << " = " << previewStart << endl;
	cout << '\t' << "PreviewEnd" << " = " << previewEnd << endl;
	cout << '\t' << "Genre" << " = " << genre << endl;
	cout << '\t' << "MediaType" << " = " << mediaType << endl;
	cout << '\t' << "MusicStream" << " = " << musicStream << endl;
	END_SECTION();

	BEGIN_SECTION(SYNC_TRACK_SECTION);
	for (auto const& evt: syncTrack) {
		cout << '\t' << evt.time << " = " << evt.type << " " << evt.value << endl;
	}
	END_SECTION();

	BEGIN_SECTION(EVENTS_SECTION);
	for (auto const& evt: events) {
		cout << '\t' << evt.time << " = E " << evt.text << endl;
	}
	END_SECTION();

	// Iterate over each note section
	for (auto const& e0: noteSections) {
		string section = e0.first;
		map<uint32_t, Note> notes = noteSections[section];

		BEGIN_SECTION(section);

		// Iterate over each note stored in this note section's map
		for (auto const& e1: notes) {
			uint32_t time = e1.first;
			Note note = notes[time];

			// Write each of the active note flags out
			for (unsigned int b = 0; b < 32; b++) {
				if ((note.value >> b) & 1) {
					cout << '\t' << note.time << " = ";
					cout << "N " << b << " ";
					// Non-playble note flags should have a duration of zero
					cout << ((b >= HOPO_FLIP_FLAG_VAL) ? 0 : note.duration) << endl;
				}
			}
		}

		END_SECTION();
	}
}
