#pragma once

#include <vector>

#include "event.h"
#include "chart.h"

namespace Fix {

	void fix_all(Chart& chart);

	/* Chart file fixes */
	void fix_missing_start_event(Chart& chart);
	void fix_missing_end_event(Chart& chart);

	/* Note track fixes */
	void fix_sustain_gap(map<uint32_t, Note>& noteTrack);
	void fix_unequal_note_durations(vector<Note>& fixed, vector<NoteEvent> simultaneousNoteEvents);

}
