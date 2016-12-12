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
#include <boost/algorithm/string/predicate.hpp>

#include "fix.h"

using namespace std;

void fix::fixAll(Chart& chart) {
	// TODO
	fixMissingStartEvent(chart);
	fixMissingEndEvent(chart);
	fixNoLeadingMeasure(chart);

	// For each note track
	for (auto it : chart.noteTracks) {
		string section = it.first;
		fixSustainGap(chart.noteTracks[section]);
	}
}

/* Chart file fixes */
void fix::fixMissingStartEvent(Chart& chart) {
	// Return if section already exists
	for (Event evt : chart.events)
		if (evt.time == 0 && boost::starts_with(evt.text, "\"section"))
			return;

	// Add a start section
	chart.events.insert(chart.events.begin(), NoteEvent(0, "\"section Start\""));
	cerr << "Inserted start section at time 0" << endl;
}

void fix::fixMissingEndEvent(Chart& chart) {
	// Return if section already exists
	for (Event evt : chart.events)
		if (evt.text == "\"end\"")
			return;

	// Find largest end time value
	string max_section;
	uint32_t max_time = 0;
	for (auto e0 : chart.noteTracks) {
		string section = e0.first;
		auto m = chart.noteTracks[section];
		auto rit = m.rbegin();
		if (rit == m.rend())
			continue; // No notes in this section
		if (max_time < rit->first) {
			max_time = rit->first; // Found new biggest time value
			max_section = section;
		}
	}

	// Add end note value and padding to end time
	Note& endNote = chart.noteTracks[max_section][max_time];
	max_time += endNote.duration;
	max_time += 100; // 100 units of padding
	chart.events.push_back(NoteEvent(max_time, "\"end\""));
	cerr << "Inserted end event at time " << max_time << endl;
}

/* Note track fixes */
void fix::fixNoLeadingMeasure(Chart& chart) {
	const unsigned int max_bpmT = 9999000; // Limit in FeedBack
	const unsigned int max_ts = 99; // Limit in FeedBack
	const unsigned int one_second_time = 8 * 48; // Offset in game time units

	if (chart.offset < 1) {
		cerr << "Cannot fix no_leading_measure: offset is less than 1" << endl;
		return;
	}

	// TODO: Don't apply if not necessary

	// Correct offset
	chart.offset -= 1; // Reduce by one second (real time units)

	// Shift all events forward (except for start event) by one second (game time units)
	// TODO: Apply also to PreviewStart/PreviewEnd IFF set
	for (SyncTrackEvent& evt : chart.syncTrack)
		evt.time += one_second_time;
	for (Event& evt : chart.events) {
		if (evt.time == 0 && boost::starts_with(evt.text, "\"section"))
			continue; // Don't move the start event
		evt.time += one_second_time;
	}
	// Shift all notes forward
	for (auto e0 : chart.noteTracks) {
		string section = e0.first;
		map<uint32_t, Note>& noteMap = chart.noteTracks[section];
		// Build new vector
		vector<Note> fixedNotes;
		for (auto e1 : noteMap) {
			Note note = noteMap[e1.first];
			note.time += one_second_time;
			fixedNotes.push_back(note);
		}
		// Clear map and rebuild with new vector
		noteMap.clear();
		for (auto note : fixedNotes)
			noteMap[note.time] = note;
	}

	// Add a single measure of 2/4 at 120BPM at the beginning of the song (= 1 second)
	chart.syncTrack.insert(chart.syncTrack.begin(), SyncTrackEvent(0, "TS", 2));
	chart.syncTrack.insert(chart.syncTrack.begin(), SyncTrackEvent(0, "B", 120000));

	cerr << "Inserted leading measure" << endl;
}

void fix::fixSustainGap(map<uint32_t, Note>& noteTrack) {
	const uint32_t min_gap = 24; // 1/32 note
	const bool apply_to_repeat_notes = false; // If the next note is identical, should the fix still be applied?
	auto it = noteTrack.begin();
	uint32_t prev_time = it->first;
	for (++it; it != noteTrack.end(); ++it) {
		Note& note = noteTrack[it->first];
		Note& prev_note = noteTrack[prev_time];
		if (prev_note.duration > 0) { // Ignore non-sustain notes
			if ((apply_to_repeat_notes && prev_note.equalsPlayable(note))
				|| !prev_note.equalsPlayable(note)) { // Ignore identical notes if set
				uint32_t prev_note_end_time = prev_note.time + prev_note.duration;
				int delta = note.time - prev_note_end_time;
				if (delta < min_gap) {
					cerr << "Sustain gap too small between " << prev_note << " and " << note << endl;
					cerr << "Duration changed from " << prev_note.duration << " to ";

					// Do fix
					delta = min_gap - delta;
					if (prev_note.duration < delta)
						delta = prev_note.duration;
					prev_note.duration -= delta;

					cerr << prev_note.duration << " (-" << delta << ")" << endl;
				}
			}
		}
		prev_time = it->first;
	}
}

void fix::fixUnequalNoteDurations(vector<Note>& fixed, vector<NoteEvent> simultaneousNoteEvents) {
	// TODO
}
