#include "fix.h"

void Fix::fix_all(Chart& chart)
{
	// TODO
	fix_missing_end_event(chart);
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

void Fix::fix_unequal_note_durations(vector<Note>& fixed, vector<NoteEvent> simultaneousNoteEvents)
{
	// TODO
}
