#pragma once

#include <vector>

#include "event.h"
#include "chart.h"

namespace fix {

	void fix_all(Chart& chart);

	/* Chart file fixes */
	void fix_missing_start_event(Chart& chart);
	void fix_missing_end_event(Chart& chart);

	/* Note track fixes */
	
	/**
	 * Fix the case where the note track(s) have no "leading measure", that is at least one blank measure
	 * before the first note. Without this, it is possible for HOPO calculations to be incorrect at the
	 * start of a song.
	 */
	void fix_no_leading_measure(Chart& chart);
	void fix_sustain_gap(map<uint32_t, Note>& noteTrack);
	void fix_unequal_note_durations(vector<Note>& fixed, vector<NoteEvent> simultaneousNoteEvents);

}
