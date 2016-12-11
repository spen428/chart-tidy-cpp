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
