#pragma once

#include <map>
#include <vector>

#include "event.h"

class Chart
{
	public:
		Chart();
		~Chart();
		bool read(char fpath[]);
		void print();

		// [Song]
		std::string name;
		std::string artist;
		std::string charter;
		double offset; // TODO
		int resolution; // TODO
		std::string player2; // TODO
		int difficulty; // TODO
		double previewStart; // TODO
		double previewEnd; // TODO
		std::string genre;
		std::string mediaType;
		std::string musicStream;
		// [SyncTrack]
		std::vector<SyncTrackEvent> syncTrack;
		// [Events]
		std::vector<Event> events;
		// Note sections
		std::map<std::string, std::multimap<uint32_t, NoteEvent>> noteSections;
	private:
		bool parse_song_line(std::string line);
		bool parse_sync_track_line(std::string line);
		bool parse_events_line(std::string line);
		bool parse_note_section_line(std::string section, std::string line);
};
