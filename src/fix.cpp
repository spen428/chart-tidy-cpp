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

#include "FeedBack.h"
#include "fix.h"

void fix::fixAll(Chart& chart) {
	// TODO
	fixMissingStartEvent(chart);
	fixMissingEndEvent(chart);
	fixNoLeadingMeasure(chart);

	// For each note track
	for (auto it : chart.noteTracks) {
		std::string section = it.first;
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
	chart.events.insert(chart.events.begin(), NoteTrackEvent(0, "\"section Start\""));
	std::cerr << "Inserted start section at time 0" << "\r\n";
}

void fix::fixMissingEndEvent(Chart& chart) {
	// Return if section already exists
	for (Event evt : chart.events)
		if (evt.text == "\"end\"")
			return;

	// Find largest end time value
	std::string max_section;
	uint32_t max_time = 0;
	for (auto e0 : chart.noteTracks) {
		std::string section = e0.first;
		std::map<uint32_t, Note> noteTrack = chart.noteTracks[section];
		auto reverseItr = noteTrack.rbegin();
		if (reverseItr == noteTrack.rend())
			continue; // No notes in this section
		if (max_time < reverseItr->first) {
			max_time = reverseItr->first; // Found new biggest time value
			max_section = section;
		}
	}

	// Add end note value and padding to end time
	Note& endNote = chart.noteTracks[max_section][max_time];
	max_time += endNote.duration;
	max_time += 100; // 100 units of padding
	chart.events.push_back(NoteTrackEvent(max_time, "\"end\""));
	std::cerr << "Inserted end event at time " << max_time << "\r\n";
}

/* Note track fixes */

void fix::fixNoLeadingMeasure(Chart& chart) {
	/**
	 * Shifts all note tracks, the sync track, and all events except for the
	 * section at time 0 forwards by 1 second, and then inserts a "leading"
	 * measure of length 1 second to the beginning of each note track.
	 * 
	 * offset value is stored to 3 d.p.
	 */

	/** Numerator of the time signature of the measure that will be inserted */
	unsigned int insert_numerator = 1;
	unsigned int insert_bpmT = 240000; // BPM * 1000 of the insert measure
	const unsigned int offset_game_time = DURATION_1_1;
	const unsigned int offset_real_time = 1; // 1 second

	// const unsigned int max_bpmT = 9999000; // Limit in FeedBack
	/// const unsigned int max_ts = 99; // Limit in FeedBack

	if (chart.offset < 1) {
		std::cerr << "Cannot fix no_leading_measure: offset is less than 1" << "\r\n";
		return;
	}

	// TODO: Don't apply if not necessary

	// Correct offset
	chart.offset -= offset_real_time; // Reduce by one second

	// Shift all events forward (except for start event) by one second (game time units)
	// TODO: Apply also to PreviewStart/PreviewEnd IFF set
	for (SyncTrackEvent& evt : chart.syncTrack)
		evt.time += offset_game_time;
	for (Event& evt : chart.events) {
		if (evt.time == 0 && boost::starts_with(evt.text, "\"section"))
			continue; // Don't move the start event
		evt.time += offset_game_time;
	}
	// Shift all notes forward
	for (auto e0 : chart.noteTracks) {
		std::string section = e0.first;
		std::map<uint32_t, Note>& noteMap = chart.noteTracks[section];
		// Build new vector
		std::vector<Note> fixedNotes;
		for (auto e1 : noteMap) {
			Note note = noteMap[e1.first];
			note.time += offset_game_time;
			fixedNotes.push_back(note);
		}
		// Clear map and rebuild with new vector
		noteMap.clear();
		for (Note note : fixedNotes) {
			noteMap[note.time] = note;
		}
	}

	// Add the insert measure
	chart.syncTrack.insert(chart.syncTrack.begin(), SyncTrackEvent(0, SYNC_TRACK_EVENT_TYPE_TIMESIG,
			insert_numerator));
	chart.syncTrack.insert(chart.syncTrack.begin(), SyncTrackEvent(0, SYNC_TRACK_EVENT_TYPE_TEMPO,
			insert_bpmT));

	std::cerr << "Inserted leading measure of " << insert_numerator << "/4 at ";
	std::cerr << (insert_bpmT / 1000) << " BPM" << "\r\n";
}

void fix::fixSustainGap(std::map<uint32_t, Note>& noteTrack) {
	const uint32_t min_gap = DURATION_1_32;
	/** If the next note is identical, should the fix still be applied? */
	const bool apply_to_repeat_notes = false;
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
					std::cerr << "Sustain gap too small between " << prev_note << " and " << note << "\r\n";
					std::cerr << "Duration changed from " << prev_note.duration << " to ";

					// Do fix
					delta = min_gap - delta;
					if (prev_note.duration < delta)
						delta = prev_note.duration;
					prev_note.duration -= delta;

					std::cerr << prev_note.duration << " (-" << delta << ")" << "\r\n";
				}
			}
		}
		prev_time = it->first;
	}
}

void fix::fixUnequalNoteDurations(std::vector<Note>& fixed, std::vector<NoteTrackEvent> simultaneousNoteEvents) {
	// TODO
}
