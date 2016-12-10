#include <iostream>
#include <set>

#include "event.h"

Event::Event(uint32_t time, std::string text):
	time(time), text(text){}

Event::~Event(){}

SyncTrackEvent::SyncTrackEvent(uint32_t time, std::string type,
		uint32_t value):
	Event(time, ""), type(type), value(value){}

SyncTrackEvent::~SyncTrackEvent(){}

NoteEvent::NoteEvent(uint32_t time, std::string text):
	Event(time, text), value(0), duration(0){}

NoteEvent::NoteEvent(uint32_t time, uint32_t value, uint32_t duration):
	Event(time, ""), value(value), duration(duration){}

NoteEvent::~NoteEvent(){}

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
	// Tap flag is the first non-playable note value
	return isNote() && value >= TAP_FLAG_VAL;
}

Note::Note(){}
Note::~Note(){}

bool Note::isTap()
{
	return (value & (1 << TAP_FLAG_VAL));
}

bool Note::isForce()
{
	return (value & (1 << HOPO_FLIP_FLAG_VAL));
}

void Note::parse_notes(map<uint32_t, Note>& noteMap, vector<NoteEvent>& simultaneousNoteEvents)
{
	Note note;
	note.time = simultaneousNoteEvents[0].time;
	note.duration = simultaneousNoteEvents[0].duration;
	note.value = 0;

	set<uint32_t> durationSet;

	// Build note bits
	for (NoteEvent evt: simultaneousNoteEvents) {
		if (!evt.isNote())
			continue;
		if (!evt.isFlag())
			durationSet.insert(evt.time);
		note.value |= (1 << evt.value);
	}

	// Assert note durations are equal
	if (!durationSet.size() == 1) {
		// Durations are not equal, must be an extended sustain. Let's fix it
		cerr << "Unequal note durations detected at time " << note.time << endl;
		// TODO
	}

	// Finally, add note to map
	noteMap[note.time] = note;
}
