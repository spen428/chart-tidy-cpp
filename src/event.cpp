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
#include <iostream>
#include <bitset>
#include <set>

#include "fix.h"
#include "event.h"

using namespace std;

Event::Event(uint32_t time, string text) :
time(time), text(text) {
}

Event::~Event() {
}

SyncTrackEvent::SyncTrackEvent(uint32_t time, string type,
		uint32_t value) :
Event(time, ""), type(type), value(value) {
}

SyncTrackEvent::~SyncTrackEvent() {
}

NoteEvent::NoteEvent(uint32_t time, string text) :
Event(time, text), value(0), duration(0) {
}

NoteEvent::NoteEvent(uint32_t time, uint32_t value, uint32_t duration) :
Event(time, ""), value(value), duration(duration) {
}

NoteEvent::~NoteEvent() {
}

bool NoteEvent::isEvent() {
	return text != "";
}

bool NoteEvent::isNote() {
	return !isEvent();
}

bool NoteEvent::isFlag() {
	// HOPO flip flag is the first non-playable note value
	return isNote() && value >= HOPO_FLIP_FLAG_VAL;
}

Note::Note() {
}

Note::~Note() {
}

bool Note::isTap() {
	return (value & (1 << TAP_FLAG_VAL));
}

bool Note::isForce() {
	return (value & (1 << HOPO_FLIP_FLAG_VAL));
}

bool Note::equalsPlayable(const Note& note) {
	return (value & note.value & 0x1F) == 1;
}

void Note::parseNotes(map<uint32_t, Note>& noteMap, vector<NoteEvent>& simultaneousNoteEvents) {
	Note note;
	note.time = simultaneousNoteEvents[0].time;
	note.duration = simultaneousNoteEvents[0].duration;
	note.value = 0;

	set<uint32_t> durationSet;

	// Build note bits
	for (NoteEvent evt : simultaneousNoteEvents) {
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
		// vector<Note> fixedNotes;
		// Fix::fix_unequal_note_durations(fixedNotes, simultaneousNoteEvents);
		// // Add notes to map
		// for (Note n: fixedNotes) {
		// 	noteMap[n.time] = n;
		// }
		noteMap[note.time] = note;
	} else {
		// Add note to map
		noteMap[note.time] = note;
	}
}

ostream& operator<<(ostream& os, const Note& n) {
	return os << "[" << n.time << ", 0b" << bitset<TOTAL_NOTE_FLAGS>(n.value) << ", " << n.duration << "]";
}
