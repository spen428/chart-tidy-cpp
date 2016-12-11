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

#include <string>
#include <stdint.h>
#include <map>
#include <vector>
#include <iostream>

#define HOPO_FLIP_FLAG_VAL 5
#define TAP_FLAG_VAL 6

const unsigned int TOTAL_NOTE_FLAGS = 7;

using namespace std;

class Event
{
	public:
		Event(uint32_t time, string text);
		~Event();

		uint32_t time;
		/**
		 * The text that appears after the "E " in the event
		 */
		string text;
};

class SyncTrackEvent: public Event
{
	public:
		SyncTrackEvent(uint32_t time, string type, uint32_t value);
		~SyncTrackEvent();
		bool isTsChange();
		bool isTempoChange();

		string type;
		uint32_t value;
};

class NoteEvent: public Event
{
	public:
		NoteEvent(uint32_t time, string text);
		NoteEvent(uint32_t time, uint32_t value, uint32_t duration);
		~NoteEvent();
		bool isNote();
		bool isFlag();
		bool isEvent();

		/**
		 * Note value as decimal
		 */
		uint32_t value;
		uint32_t duration;
};

class Note
{
	public:
		Note();
		~Note();
		bool isTap();
		bool isForce();
		/**
		 * True if the first 5 bits of each Note's `value` is the same, signifying that they are the same note,
		 * ignoring tap/force status.
		 */
		bool equalsPlayable(const Note& note);

		friend ostream& operator<<(ostream& os, const Note& n);

		static void parse_notes(map<uint32_t, Note>& noteMap, vector<NoteEvent>& simultaneousNoteEvents);

		uint32_t time;
		/**
		 * Note values as bits
		 */
		uint32_t value;
		uint32_t duration;
};
