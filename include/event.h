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

const unsigned int NOTE_FLAG_VAL_GREEN = 0;
const unsigned int NOTE_FLAG_VAL_RED = 1;
const unsigned int NOTE_FLAG_VAL_YELLOW = 2;
const unsigned int NOTE_FLAG_VAL_BLUE = 3;
const unsigned int NOTE_FLAG_VAL_ORANGE = 4;
const unsigned int NOTE_FLAG_VAL_HOPO_FLIP = 5;
const unsigned int NOTE_FLAG_VAL_TAP = 6;
const unsigned int NOTE_FLAG_TOTAL = 7;
const unsigned int PLAYABLE_NOTE_TOTAL = 5; // GRYBO

const std::string NOTE_TRACK_EVENT_TYPE_EVENT = "E";
const std::string NOTE_TRACK_EVENT_TYPE_STAR_POWER = "S";
const std::string NOTE_TRACK_EVENT_TYPE_NOTE = "N";

const std::string SYNC_TRACK_EVENT_TYPE_TEMPO = "B";
const std::string SYNC_TRACK_EVENT_TYPE_TIMESIG = "TS";

class Event {
public:
    Event(uint32_t time, std::string text);
    Event(uint32_t time, std::string type, std::string text);
    ~Event();
    /**
     * Return the string representation of this event as it would appear in a
     * chart file. Subclasses of `Event` should use this method to allow for
     * easy serialisation.
     */
    virtual std::string toEventString() const;

    uint32_t time;
    /**
     * The "E" in the event string
     */
    std::string type;
    /**
     * The text that appears after the "E " in the event
     */
    std::string text;
};

class SyncTrackEvent : public Event {
public:
    SyncTrackEvent(uint32_t time, std::string type, uint32_t value);
    ~SyncTrackEvent();
    std::string toEventString() const override;
    bool isTsChange() const;
    bool isTempoChange() const;

    uint32_t value;
};

class NoteTrackEvent : public Event {
public:
    /**
     * Constructor for E-type events in the note track
     */
    NoteTrackEvent(uint32_t time, std::string text);
    /**
     * Constructor for N-type events in the note track
     */
    NoteTrackEvent(uint32_t time, uint32_t value, uint32_t duration);
    /**
     * Constructor for arbitrary-type events in the note track
     */
    NoteTrackEvent(uint32_t time, std::string type, uint32_t value, uint32_t duration);
    ~NoteTrackEvent();
    std::string toEventString() const override;
    bool isNote() const;
    bool isFlag() const;
    bool isEvent() const;
    bool isStarPower() const;

    /**
     * Note value as decimal
     */
    uint32_t value;
    uint32_t duration;
};

class Note {
public:
    Note();
    ~Note();
    bool isTap() const;
    bool isForce() const;
    /**
     * True if the first 5 bits of each Note's `value` is the same, signifying that they are the same note,
     * ignoring tap/force status.
     */
    bool equalsPlayable(const Note& note) const;
    /**
     * Convert this `Note` into a set of `NoteTrackEvent`s and add it to the given vector.
     */
    void toNoteTrackEvents(std::vector<NoteTrackEvent>& vector);

    friend std::ostream& operator<<(std::ostream& os, const Note& n);

    uint32_t time;
    /**
     * Note values as bits
     */
    uint32_t value;
    uint32_t duration;
};
