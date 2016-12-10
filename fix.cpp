#include <iostream>

#include "fix.h"

using namespace std;

void Fix::fix_all(Chart& chart)
{
	// TODO
	fix_missing_end_event(chart);

	// For each note track
	for (auto it: chart.noteSections) {
		string section = it.first;
		fix_sustain_gap(chart.noteSections[section]);
	}
}

/* Chart file fixes */
void Fix::fix_missing_end_event(Chart& chart)
{
	// Return if end section already exists
	for (Event evt: chart.events)
		if (evt.text == "\"end\"")
			return;

	// Find largest end time value
	string max_section;
	uint32_t max_time = 0;
	for (auto e0: chart.noteSections) {
		string section = e0.first;
		auto m = chart.noteSections[section];
		auto rit = m.rbegin();
		if (rit == m.rend())
			continue; // No notes in this section
		if (max_time < rit->first) {
			max_time = rit->first; // Found new biggest time value
			max_section = section;
		}
	}

	// Add end note value and padding to end time
	Note& endNote = chart.noteSections[max_section][max_time];
	max_time += endNote.duration;
	max_time += 100; // 100 units of padding
	chart.events.push_back(NoteEvent(max_time, "\"end\""));
}

/* Note track fixes */
void Fix::fix_sustain_gap(map<uint32_t, Note>& noteTrack)
{
	const uint32_t min_gap = 48; // 1/16 note
	auto it = noteTrack.begin();
	Note& prev_note = noteTrack[it->first];
	for (++it; it != noteTrack.end(); it++) {
		Note& note = noteTrack[it->first];
		if (prev_note.duration > 0) {
			uint32_t prev_note_end_time = prev_note.time + prev_note.duration;
			int delta = note.time - prev_note_end_time;
			if (delta < min_gap) {
				cerr << "Sustain gap too small between " << prev_note << " and " << note << ". ";
				cerr << "Set " << prev_note << " => ";

				// Do fix
				delta = min_gap - delta;
				if (prev_note.duration < delta)
					delta = prev_note.duration;
				prev_note.duration -= delta;

				cerr << prev_note << " (-" << delta << ")" << endl;
			}
		}
		prev_note = note;
	}
}

void Fix::fix_unequal_note_durations(vector<Note>& fixed, vector<NoteEvent> simultaneousNoteEvents)
{
	// TODO
}
