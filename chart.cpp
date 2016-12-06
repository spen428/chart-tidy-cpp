#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "chart.h"
#include "event.h"

#define SONG_SECTION "Song"
#define SYNC_TRACK_SECTION "SyncTrack"
#define EVENTS_SECTION "Events"

using namespace std;

void split_once(string& first, string& second, string str);
bool is_note_section(string section);

Chart::Chart()
{
}

Chart::~Chart()
{
}

bool Chart::read(char *fpath)
{
	bool errors = false;
	bool in_block = false;
	ifstream infile(fpath);
	string section = "";

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
					if (parse_note_section_line(section, line))
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
	return !errors;
}

/**
 * Split a string `str` into two substrings `first` and `second` and trim the
 * whitespace from them. If `delim` is not found in `str`, `first` will equal
 * `str` after trimming and `second` will equal "".
 */
void split_once(string& first, string& second,
		string str, char delim)
{
	const auto idx = str.find_first_of(delim);
	if (string::npos != idx) {
		first = str.substr(0, idx);
		second = str.substr(idx + 1);
	} else {
		first = str;
		second = "";
	}
	boost::trim(first);
	boost::trim(second);
}

bool Chart::parse_song_line(string line)
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
	} else if (key == "Resolution") {
	} else if (key == "Player2") {
	} else if (key == "Difficulty") {
	} else if (key == "PreviewStart") {
	} else if (key == "PreviewEnd") {
	} else if (key == "Genre") {
	} else if (key == "MediaType") {
	} else if (key == "MusicStream") {
		return false; // TODO
	} else {
		cerr << "Unknown key: " << key << endl;
		return false;
	}
	return true;
}

bool Chart::parse_sync_track_line(string line)
{
	// TODO
	return false;
}

bool Chart::parse_events_line(string line)
{
	// TODO
	return false;
}

bool Chart::parse_note_section_line(string section, string line)
{
	// TODO
//	bool errors = false;
//	istringstream ss(line);
//	char prefix = line.at(0);
//	if (prefix == 'N') {
//		uint32_t value;
//		uint32_t duration;
//		ss >> prefix >> value >> duration;
//		NoteEvent evt(time, value, duration);
//		noteSections.insert(pair(time, evt));
//	} else if (prefix == 'E') {
//		string text;
//		ss >> prefix >> text;
//		return NoteEvent(time, text);
//	} else {
//		cerr << "Invalid note event string: " << line << endl;
//		errors = true;
//	}
//	return !errors;
	return false;
}

bool is_note_section(string section)
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
#define KV(X, Y) cout << '\t' << X << " = " << Y << endl
void Chart::print()
{
	BEGIN_SECTION(SONG_SECTION);
	KV("Name", name);
	KV("Artist", artist);
	KV("Charter", charter);
	KV("Offset", offset);
	KV("Resolution", resolution);
	KV("Player2", player2);
	KV("Difficulty", difficulty);
	KV("PreviewStart", previewStart);
	KV("PreviewEnd", previewEnd);
	KV("Genre", genre);
	KV("MediaType", mediaType);
	KV("MusicStream", musicStream);
	END_SECTION();

	BEGIN_SECTION(SYNC_TRACK_SECTION);
	// TODO
	END_SECTION();

	BEGIN_SECTION(EVENTS_SECTION);
	// TODO
	END_SECTION();

	// TODO
	//for (auto section: ...) {
	//	BEGIN_SECTION(section);
	//	KV(key, value);
	//	END_SECTION();
	//}
}
