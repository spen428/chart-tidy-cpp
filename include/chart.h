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
#include <unordered_map>
#include <set>
#include <vector>

#include "event.h"

class Chart {
public:
    Chart();
    ~Chart();
    bool read(char fpath[]);
    void print();

    // [Song]
    std::string name;
    std::string artist;
    std::string charter;
    double offset;
    int resolution;
    std::string player2;
    int difficulty;
    double previewStart;
    double previewEnd;
    std::string genre;
    std::string mediaType;
    std::string musicStream;
    // [SyncTrack]
    std::vector<SyncTrackEvent> syncTrack;
    // [Events]
    std::vector<Event> events;
    // Note tracks, e.g. [ExpertSingle]
    /** Playable notes and note flags, i.e. anything starting with "N" in a note track */
    std::unordered_map<std::string, std::map<uint32_t, Note>> noteTrackNotes;
    /** Everything else that appears in a note track: star power and track events */
    std::unordered_map<std::string, std::vector<NoteTrackEvent>> noteTrackEvents;
private:
    bool parseSongLine(const std::string& line);
    bool parseSyncTrackLine(const std::string& line);
    bool parseEventsLine(const std::string& line);
    bool parseNoteSectionLine(const std::string& section, const std::string& line);
    bool extractNotesFromNoteTrackEvents();
    /**
     * Merge std::vector<NoteTrackEvent> and std::map<uint32_t, Note> into
     * a single std::vector<NoteTrackEvent>, converting all Note objects.
     */
    void mergeEvents(std::vector<NoteTrackEvent>& out, const std::vector<NoteTrackEvent>& nte, const std::map<uint32_t, Note>& notes);
};
