#include "event.h"

Event::Event(uint32_t time, std::string text):
	time(time), text(text)
{
}

Event::~Event()
{
}

SyncTrackEvent::SyncTrackEvent(uint32_t time, std::string type,
		uint32_t value):
	Event(time, ""), type(type), value(value)
{
}

SyncTrackEvent::~SyncTrackEvent()
{
}

NoteEvent::NoteEvent(uint32_t time, std::string text):
	Event(time, text), value(0), duration(0)
{
}

NoteEvent::NoteEvent(uint32_t time, uint32_t value, uint32_t duration):
	Event(time, ""), value(value), duration(duration)
{
}

NoteEvent::~NoteEvent()
{
}

bool NoteEvent::isEvent()
{
	return text != "";
}

bool NoteEvent::isNote()
{
	return !isEvent();
}

bool NoteEvent::isFlag()
{
	return isNote() && value >= NUM_PLAYABLE_NOTES;
}

bool NoteEvent::isPlayable()
{
	return isNote() && value < NUM_PLAYABLE_NOTES;
}
