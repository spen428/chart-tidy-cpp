#pragma once

#include <map>
#include <vector>

#include "event.h"

using namespace std;

class Chart
{
	public:
		Chart();
		~Chart();
		bool read(char fpath[]);
		void print();

		// [Song]
		string name;
		string artist;
		string charter;
		double offset;
		int resolution;
		string player2;
		int difficulty;
		double previewStart;
		double previewEnd;
		string genre;
		string mediaType;
		string musicStream;
		// [SyncTrack]
		vector<SyncTrackEvent> syncTrack;
		// [Events]
		vector<Event> events;
		// Note sections
		map<string, multimap<uint32_t, NoteEvent>> noteSections;
	private:
		bool parse_song_line(const string& line);
		bool parse_sync_track_line(const string& line);
		bool parse_events_line(const string& line);
		bool parse_note_section_line(const string& section,
				const string& line);
};
