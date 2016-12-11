#pragma once

#include <map>
#include <unordered_map>
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
		void print(bool feedback_safe);

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
		unordered_map<string, map<uint32_t, Note>> noteSections;
	private:
		bool parse_song_line(const string& line);
		bool parse_sync_track_line(const string& line);
		bool parse_events_line(const string& line);
		bool parse_note_section_line(map<uint32_t, vector<NoteEvent>>& noteEvents, const string& line);
		bool parse_note_events(unordered_map<string, map<uint32_t, vector<NoteEvent>>>& noteEvents);
};
