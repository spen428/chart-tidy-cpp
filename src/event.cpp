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

bool operator<(const Event& e0, const Event& e1) {
	if (e0.time < e1.time)
		return true;
	if (e0.time == e1.time) {
		if (e0.type < e1.type)
			return true;
		if (e0.type == e1.type)
			return (e0.text < e1.text);
	}
	return false;
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

bool operator<(const SyncTrackEvent& e0, const SyncTrackEvent& e1) {
	if (e0.time < e1.time)
		return true;
	if (e0.time == e1.time) {
		if (e0.type < e1.type)
			return true;
		if (e0.type == e1.type)
			return (e0.value < e1.value);
	}
	return false;
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
	return isNote() && value > NOTE_FLAG_VAL_ORANGE;
}

bool NoteTrackEvent::isStarPower() const {
	return type == NOTE_TRACK_EVENT_TYPE_STAR_POWER;
}

bool operator<(const NoteTrackEvent& nte0, const NoteTrackEvent& nte1) {
	if (nte0.time < nte1.time)
		return true;
	if (nte0.time == nte1.time) {
		if (nte0.type < nte1.type)
			return true;
		if (nte0.type == nte1.type) {
			if (nte0.text < nte1.text)
				return true;
			if (nte0.text == nte1.text)
				return (nte0.value < nte1.value);
		}
	}
	return false;
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

void Note::toNoteTrackEvents(std::vector<NoteTrackEvent>& vec) const {
	// Write each of the active note flags out
	for (unsigned int b = 0; b < 32; b++) {
		if (!((value >> b) & 1))
			continue;
		// Set duration to 0 for non-playable note flags
		if (b > NOTE_FLAG_VAL_ORANGE && b != NOTE_FLAG_VAL_OPEN)
			vec.push_back(NoteTrackEvent(time, b, 0));
		else
			vec.push_back(NoteTrackEvent(time, b, duration));
	}
}

bool operator<(const Note& n0, const Note& n1) {
	if (n0.time < n1.time)
		return true;
	if (n0.time == n1.time)
		return (n0.value < n1.value);
	return false;
}

std::ostream& operator<<(std::ostream& os, const Note& note) {
	return os << "[" << note.time << ", 0b" << std::bitset<NOTE_FLAG_TOTAL>(note.value) << ", " << note.duration << "]";
}
