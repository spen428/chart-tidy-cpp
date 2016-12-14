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

#include <map>
#include <vector>

#include "event.h"
#include "chart.h"

namespace fix {

    void fixAll(Chart& chart);

    /* Chart file fixes */
    void fixMissingStartEvent(Chart& chart);
    void fixMissingEndEvent(Chart& chart);
    void fixUnprintableCharacters(Chart& chart);

    /* Note track fixes */

    /**
     * Replace event markers with tap notes / force notes.
     */
    void setNoteFlags(Chart& chart);
    /**
     * Replace tap notes / force notes with event markers.
     */
    void unsetNoteFlags(Chart& chart);
    /**
     * Fix the case where the note track(s) have no "leading measure", that is at least one blank measure
     * before the first note. Without this, it is possible for HOPO calculations to be incorrect at the
     * start of a song.
     */
    void fixNoLeadingMeasure(Chart& chart);
    void fixSustainGap(std::map<uint32_t, Note>& noteTrack, const unsigned int min_gap);
    void fixUnequalNoteDurations(std::vector<Note>& fixed, std::vector<NoteTrackEvent> simultaneousNoteEvents);
    /**
     * Automatically inserts star power phrases into the chart.
     */
    void fixMissingStarPower(Chart& chart);

}
