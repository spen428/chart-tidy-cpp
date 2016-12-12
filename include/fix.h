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
