#pragma once

#include <string>
#include <stdint.h>

#define NUM_PLAYABLE_NOTES 5

class Event
{
	public:
		Event(uint32_t time, std::string text);
		~Event();
		uint32_t time;
		/**
		 * The text that appears after the "E " in the event
		 */
		std::string text;
};

class SyncTrackEvent: public Event
{
	public:
		SyncTrackEvent(uint32_t time, std::string type, uint32_t value);
		~SyncTrackEvent();
		bool isTsChange();
		bool isTempoChange();
		std::string type;
		uint32_t value;
};


class NoteEvent: public Event
{
	public:
		NoteEvent(uint32_t time, std::string text);
		NoteEvent(uint32_t time, uint32_t value, uint32_t duration);
		~NoteEvent();
		/**
		 * Returns true if this Note object is a playable note, i.e. is
		 * a Green, Red, Yellow, Blue, or Orange note.
		 */
		bool isPlayable();
		/**
		 * Returns true if this Note object is a note flag, i.e. the tap
		 * flag or the HOPO flag.
		 */
		bool isFlag();
		bool isNote();
		bool isEvent();
		uint32_t value;
		uint32_t duration;
};
