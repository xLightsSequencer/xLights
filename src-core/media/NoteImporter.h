#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Wx-free note importers shared by the desktop "Import Notes" dialog
// and the iPad timing-track import sheet. Each loader returns a
// time(ms) -> list-of-midi-notes map quantised to `intervalMS`, the
// same shape the desktop xLightsFrame loaders produced. The MIDI
// reader is a self-contained Standard MIDI File parser (no external
// midifile lib) so the iPad core target can use it without pulling in
// the desktop-only `smf` library. MusicXML parsing handles plain
// (uncompressed) `.musicxml` / `.xml` documents via pugixml; the
// zipped `.mxl` container stays a desktop-only path.

#include <map>
#include <string>
#include <vector>

namespace NoteImporter {

// Audacity label/timing text export: tab-separated "start<TAB>end<TAB>midi"
// lines (one note per line).
std::map<int, std::vector<float>> LoadAudacityFile(const std::string& file, int intervalMS);

// Uncompressed MusicXML (`score-partwise`). `track` is a part name as
// returned by MusicXMLTracks(); "All" merges every part.
std::map<int, std::vector<float>> LoadMusicXMLFile(const std::string& file, int intervalMS,
                                                   int speedAdjustPct, int startAdjustMS,
                                                   const std::string& track);

// Standard MIDI File (format 0/1). `track` is a 1-based track index as
// a string, or "All"/"" to merge all tracks.
std::map<int, std::vector<float>> LoadMIDIFile(const std::string& file, int intervalMS,
                                               int speedAdjustPct, int startAdjustMS,
                                               const std::string& track);

// Part / track names available for a MusicXML or MIDI file (used to
// populate the track picker). Returns an empty list if the file can't
// be read or has no named tracks.
std::vector<std::string> MusicXMLTracks(const std::string& file);
int MIDITrackCount(const std::string& file);

// Convert a time->notes map into a flat list of [startMS, endMS, label]
// timing marks (comma-separated note names per mark, blank where no
// note plays), spanning [0, durationMS]. Mirrors xLightsFrame::CreateNotes.
struct NoteMark {
    int startMS;
    int endMS;
    std::string label;
};
std::vector<NoteMark> BuildNoteMarks(const std::map<int, std::vector<float>>& notes,
                                     int intervalMS, int durationMS);

} // namespace NoteImporter
