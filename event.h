#pragma once

#include <string>
#include <stdint.h>
#include <map>
#include <vector>
#include <iostream>

#define HOPO_FLIP_FLAG_VAL 5
#define TAP_FLAG_VAL 6

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

		friend ostream& operator<<(ostream& os, const Note& n);

		static void parse_notes(map<uint32_t, Note>& noteMap, vector<NoteEvent>& simultaneousNoteEvents);

		uint32_t time;
		/**
		 * Note values as bits
		 */
		uint32_t value;
		uint32_t duration;
};
