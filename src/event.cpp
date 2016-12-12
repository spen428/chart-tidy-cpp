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
#include <sstream>
#include <bitset>
#include <set>

#include "fix.h"
#include "event.h"

Event::Event(uint32_t time, std::string text) :
Event(time, "E", text) {
}

Event::Event(uint32_t time, std::string type, std::string text) :
time(time), type(type), text(text) {
}

Event::~Event() {
}

std::string Event::toEventString() const {
	std::stringstream ss;
	ss << time << " = " << type << " " << text;
	return ss.str();
}

SyncTrackEvent::SyncTrackEvent(uint32_t time, std::string type,
		uint32_t value) :
Event(time, type, ""), value(value) {
}

SyncTrackEvent::~SyncTrackEvent() {
}

std::string SyncTrackEvent::toEventString() const {
	std::stringstream ss;
	ss << time << " = " << type << " " << value;
	return ss.str();
}

bool SyncTrackEvent::isTempoChange() const {
	return type == SYNC_TRACK_EVENT_TYPE_TEMPO;
}

bool SyncTrackEvent::isTsChange() const {
	return type == SYNC_TRACK_EVENT_TYPE_TIMESIG;
}



NoteTrackEvent::NoteTrackEvent(uint32_t time, std::string text) :
Event(time, text), value(0), duration(0) {
}

NoteTrackEvent::NoteTrackEvent(uint32_t time, uint32_t value, uint32_t duration) :
NoteTrackEvent(time, NOTE_TRACK_EVENT_TYPE_NOTE, value, duration) {
}

NoteTrackEvent::NoteTrackEvent(uint32_t time, std::string type, uint32_t value, uint32_t duration) :
Event(time, type, ""), value(value), duration(duration) {
}

NoteTrackEvent::~NoteTrackEvent() {
}

std::string NoteTrackEvent::toEventString() const {
	if (isEvent()) {
		return Event::toEventString();
	}
	std::stringstream ss;
	ss << time << " = " << type << " " << value << " " << duration;
	return ss.str();
}

bool NoteTrackEvent::isEvent() const {
	return type == NOTE_TRACK_EVENT_TYPE_EVENT;
}

bool NoteTrackEvent::isNote() const {
	return type == NOTE_TRACK_EVENT_TYPE_NOTE;
}

bool NoteTrackEvent::isFlag() const {
	return isNote() && value >= PLAYABLE_NOTE_TOTAL;
}

bool NoteTrackEvent::isStarPower() const {
	return type == NOTE_TRACK_EVENT_TYPE_STAR_POWER;
}

Note::Note() {
}

Note::~Note() {
}

bool Note::isTap() const {
	return (value & (1 << NOTE_FLAG_VAL_TAP));
}

bool Note::isForce() const {
	return (value & (1 << NOTE_FLAG_VAL_HOPO_FLIP));
}

bool Note::equalsPlayable(const Note& note) const {
	return (value & note.value & 0x1F) == 0x1F;
}

void Note::parseNotes(std::map<uint32_t, Note>& noteMap, std::vector<NoteTrackEvent>& simultaneousNoteEvents) {
	Note note;
	note.time = simultaneousNoteEvents[0].time;
	note.duration = simultaneousNoteEvents[0].duration;
	note.value = 0;

	std::set<uint32_t> durationSet;

	// Build note bits
	for (NoteTrackEvent evt : simultaneousNoteEvents) {
		if (!evt.isNote())
			continue;
		if (!evt.isFlag())
			durationSet.insert(evt.time);
		note.value |= (1 << evt.value);
	}

	// Assert note durations are equal
	if (!durationSet.size() == 1) {
		// Durations are not equal, must be an extended sustain. Let's fix it
		std::cerr << "Unequal note durations detected at time " << note.time << "\r\n";
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

void Note::toNoteTrackEvents(std::vector<NoteTrackEvent>& vector) {

}

std::ostream& operator<<(std::ostream& os, const Note& note) {
	return os << "[" << note.time << ", 0b" << std::bitset<NOTE_FLAG_TOTAL>(note.value) << ", " << note.duration << "]";
}
