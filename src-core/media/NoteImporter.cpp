/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "NoteImporter.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <list>

#include <pugixml.hpp>
#include <log.h>

#include "utils/ExternalHooks.h"
#include "utils/UtilFunctions.h"

namespace {

// Round t (seconds) to the nearest interval boundary, in ms. Matches
// the desktop LowerTS/UpperTS (which were identical nearest-rounding).
int RoundTS(float tSeconds, int intervalMS) {
    if (intervalMS <= 0) intervalMS = 1;
    int res = (int)(tSeconds * 1000.0f);
    res += intervalMS / 2;
    res /= intervalMS;
    res *= intervalMS;
    return res;
}

void AddNoteAt(std::map<int, std::vector<float>>& res, int frameMS, float midi) {
    auto it = res.find(frameMS);
    if (it == res.end()) {
        res[frameMS] = std::vector<float>{ midi };
        return;
    }
    for (float v : it->second) {
        if (v == midi) return;
    }
    it->second.push_back(midi);
}

// ---- Minimal Standard MIDI File reader -----------------------------

struct MidiReader {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;

    bool eof() const { return p >= end; }
    uint8_t u8() { return eof() ? 0 : *p++; }
    uint32_t u16() {
        uint32_t v = (uint32_t)u8() << 8;
        v |= u8();
        return v;
    }
    uint32_t u32() {
        uint32_t v = (uint32_t)u8() << 24;
        v |= (uint32_t)u8() << 16;
        v |= (uint32_t)u8() << 8;
        v |= u8();
        return v;
    }
    uint32_t vlq() {
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t b = u8();
            v = (v << 7) | (b & 0x7F);
            if (!(b & 0x80)) break;
        }
        return v;
    }
};

struct MidiNoteEvent {
    int trackIndex;
    double timeSeconds;
    int key;
    bool noteOn; // true = on (vel>0), false = off
};

// Parse the whole file into per-track note on/off events with absolute
// times in seconds (honouring tempo changes). Returns false if not a
// valid SMF. trackCount receives the number of tracks.
bool ParseMIDI(const std::string& file, std::vector<MidiNoteEvent>& events, int& trackCount) {
    std::ifstream f(file, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (buf.size() < 14) return false;

    MidiReader r{ buf.data(), buf.data() + buf.size() };
    if (r.u8() != 'M' || r.u8() != 'T' || r.u8() != 'h' || r.u8() != 'd') return false;
    uint32_t hdrLen = r.u32();
    /*uint32_t format =*/ r.u16();
    uint32_t ntracks = r.u16();
    int division = (int)r.u16();
    // skip any extra header bytes
    for (uint32_t i = 6; i < hdrLen; ++i) r.u8();

    if (division & 0x8000) {
        // SMPTE time division (frames per second / ticks per frame) —
        // uncommon for these imports; bail rather than mis-time it.
        division = 480;
    }
    if (division == 0) division = 480;

    trackCount = (int)ntracks;

    for (uint32_t t = 0; t < ntracks; ++t) {
        if (r.eof()) break;
        // Track chunk header "MTrk" + length
        if (r.u8() != 'M' || r.u8() != 'T' || r.u8() != 'r' || r.u8() != 'k') break;
        uint32_t len = r.u32();
        const uint8_t* trackEnd = r.p + len;
        if (trackEnd > r.end) trackEnd = r.end;

        MidiReader tr{ r.p, trackEnd };
        uint32_t absTicks = 0;
        uint8_t runningStatus = 0;
        double microsPerQuarter = 500000.0; // default 120 bpm
        double secondsAccum = 0.0;
        uint32_t lastTickForTime = 0;

        auto ticksToSeconds = [&](uint32_t toTick) {
            uint32_t dt = toTick - lastTickForTime;
            secondsAccum += (double)dt * (microsPerQuarter / 1000000.0) / (double)division;
            lastTickForTime = toTick;
            return secondsAccum;
        };

        while (!tr.eof()) {
            uint32_t delta = tr.vlq();
            absTicks += delta;
            uint8_t status = tr.u8();
            if (status < 0x80) {
                // running status: reuse last, and `status` was actually data
                tr.p--;
                status = runningStatus;
            } else {
                runningStatus = status;
            }

            uint8_t hi = status & 0xF0;
            if (hi == 0x90 || hi == 0x80) { // note on / off
                uint8_t key = tr.u8();
                uint8_t vel = tr.u8();
                double secs = ticksToSeconds(absTicks);
                bool on = (hi == 0x90) && (vel > 0);
                events.push_back({ (int)t, secs, (int)key, on });
            } else if (hi == 0xA0 || hi == 0xB0 || hi == 0xE0) {
                tr.u8(); tr.u8(); // two data bytes
            } else if (hi == 0xC0 || hi == 0xD0) {
                tr.u8(); // one data byte
            } else if (status == 0xFF) { // meta
                uint8_t type = tr.u8();
                uint32_t mlen = tr.vlq();
                if (type == 0x51 && mlen == 3) { // set tempo
                    ticksToSeconds(absTicks); // settle accumulated time at old tempo
                    uint32_t mpq = (uint32_t)tr.u8() << 16;
                    mpq |= (uint32_t)tr.u8() << 8;
                    mpq |= tr.u8();
                    microsPerQuarter = (double)mpq;
                } else {
                    for (uint32_t i = 0; i < mlen; ++i) tr.u8();
                }
            } else if (status == 0xF0 || status == 0xF7) { // sysex
                uint32_t slen = tr.vlq();
                for (uint32_t i = 0; i < slen; ++i) tr.u8();
            } else {
                // unknown — stop this track to avoid desync
                break;
            }
        }
        r.p = trackEnd;
    }
    return !events.empty();
}

// ---- MusicXML (uncompressed) ---------------------------------------

struct MxNote {
    int midi = 0;
    int start = 0;
    int duration = 0;
    bool rest() const { return midi == -1; }
};

int PitchToMidi(pugi::xml_node pitch) {
    std::string step;
    int octave = 4, alter = 0;
    for (pugi::xml_node n = pitch.first_child(); n; n = n.next_sibling()) {
        std::string_view nm(n.name());
        if (nm == "step") step = n.text().get();
        else if (nm == "octave") octave = n.text().as_int();
        else if (nm == "alter") alter = n.text().as_int();
    }
    int base = 0;
    if (step == "A") base = 9;
    else if (step == "B") base = 11;
    else if (step == "C") base = 0;
    else if (step == "D") base = 2;
    else if (step == "E") base = 4;
    else if (step == "F") base = 5;
    else if (step == "G") base = 7;
    return base + alter + (octave + 1) * 12;
}

bool LoadMusicXMLDoc(const std::string& file, pugi::xml_document& doc) {
    if (file.empty() || !FileExists(file)) return false;
    return (bool)doc.load_file(file.c_str());
}

} // namespace

namespace NoteImporter {

std::map<int, std::vector<float>> LoadAudacityFile(const std::string& file, int intervalMS) {
    std::map<int, std::vector<float>> res;
    if (file.empty() || !FileExists(file)) return res;
    std::ifstream f(file);
    if (!f) return res;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        // tab-separated start, end, midi
        std::vector<float> comp;
        size_t pos = 0;
        while (pos <= line.size()) {
            size_t tab = line.find('\t', pos);
            std::string tok = (tab == std::string::npos) ? line.substr(pos) : line.substr(pos, tab - pos);
            comp.push_back((float)std::strtod(tok.c_str(), nullptr));
            if (tab == std::string::npos) break;
            pos = tab + 1;
        }
        if (comp.size() != 3) {
            spdlog::warn("Invalid data in audacity file - 3 tab separated values expected: '{}'", line);
            break;
        }
        int start = RoundTS(comp[0], intervalMS);
        int end = RoundTS(comp[1], intervalMS);
        for (int i = start; i < end; i += intervalMS) {
            AddNoteAt(res, i, comp[2]);
        }
    }
    return res;
}

std::map<int, std::vector<float>> LoadMusicXMLFile(const std::string& file, int intervalMS,
                                                   int speedAdjustPct, int startAdjustMS,
                                                   const std::string& track) {
    std::map<int, std::vector<float>> res;
    pugi::xml_document doc;
    if (!LoadMusicXMLDoc(file, doc)) {
        spdlog::warn("Invalid / unsupported MusicXML file {}", file);
        return res;
    }

    pugi::xml_node root = doc.document_element();
    if (std::string_view(root.name()) != "score-partwise") return res;

    // resolve part id for the requested track name
    std::string partid = "All";
    if (track != "All" && !track.empty()) {
        for (pugi::xml_node n1 = root.first_child(); n1; n1 = n1.next_sibling()) {
            if (std::string_view(n1.name()) != "part-list") continue;
            for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling()) {
                if (std::string_view(n2.name()) != "score-part") continue;
                std::string id = n2.attribute("id").as_string();
                for (pugi::xml_node n3 = n2.first_child(); n3; n3 = n3.next_sibling()) {
                    if (std::string_view(n3.name()) == "part-name") {
                        std::string pn = n3.text().get();
                        if (pn == track || (pn.empty() && id == track)) partid = id;
                    }
                }
            }
            break;
        }
    }

    int tempo = 60;
    int divisions = 100;
    std::list<MxNote> notes;

    for (pugi::xml_node n1 = root.first_child(); n1; n1 = n1.next_sibling()) {
        if (std::string_view(n1.name()) != "part") continue;
        if (track != "All" && std::string_view(n1.attribute("id").as_string()) != partid) continue;
        int current = 0;
        for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling()) {
            if (std::string_view(n2.name()) != "measure") continue;
            for (pugi::xml_node n3 = n2.first_child(); n3; n3 = n3.next_sibling()) {
                std::string_view nm(n3.name());
                if (nm == "direction") {
                    for (pugi::xml_node n4 = n3.first_child(); n4; n4 = n4.next_sibling()) {
                        if (std::string_view(n4.name()) == "sound" && n4.attribute("tempo"))
                            tempo = n4.attribute("tempo").as_int();
                    }
                } else if (nm == "attributes") {
                    for (pugi::xml_node n4 = n3.first_child(); n4; n4 = n4.next_sibling()) {
                        if (std::string_view(n4.name()) == "divisions")
                            divisions = n4.text().as_int();
                    }
                } else if (nm == "note") {
                    MxNote note;
                    note.start = current;
                    for (pugi::xml_node nn = n3.first_child(); nn; nn = nn.next_sibling()) {
                        std::string_view nnm(nn.name());
                        if (nnm == "pitch") note.midi = PitchToMidi(nn);
                        else if (nnm == "duration") note.duration = nn.text().as_int();
                        else if (nnm == "rest") note.midi = -1;
                    }
                    current += note.duration;
                    notes.push_back(note);
                }
            }
        }
    }

    if (tempo == 0) tempo = 60;
    if (divisions == 0) divisions = 1;
    float msPerDuration = ((60.0f * 1000.0f) / tempo) / (4.0f * divisions);
    float speedadjust = speedAdjustPct / 100.0f;

    for (const MxNote& note : notes) {
        if (note.rest()) continue;
        float startMS = (float)note.start * msPerDuration;
        float durMS = (float)note.duration * msPerDuration;
        float startt = (float)startAdjustMS / 100.0f + startMS / 1000.0f * speedadjust;
        float endt = (float)startAdjustMS / 100.0f + (startMS + durMS) / 1000.0f * speedadjust;
        int start = RoundTS(startt, intervalMS);
        int end = RoundTS(endt, intervalMS);
        for (int i = start; i < end; i += intervalMS) {
            if (i >= 0) AddNoteAt(res, i, (float)note.midi);
        }
    }
    return res;
}

std::vector<std::string> MusicXMLTracks(const std::string& file) {
    std::vector<std::string> res;
    pugi::xml_document doc;
    if (!LoadMusicXMLDoc(file, doc)) return res;
    pugi::xml_node root = doc.document_element();
    if (std::string_view(root.name()) != "score-partwise") return res;
    for (pugi::xml_node n1 = root.first_child(); n1; n1 = n1.next_sibling()) {
        if (std::string_view(n1.name()) != "part-list") continue;
        for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling()) {
            if (std::string_view(n2.name()) != "score-part") continue;
            std::string id = n2.attribute("id").as_string();
            for (pugi::xml_node n3 = n2.first_child(); n3; n3 = n3.next_sibling()) {
                if (std::string_view(n3.name()) == "part-name") {
                    std::string pn = n3.text().get();
                    res.push_back(pn.empty() ? id : pn);
                }
            }
        }
        break;
    }
    return res;
}

std::map<int, std::vector<float>> LoadMIDIFile(const std::string& file, int intervalMS,
                                               int speedAdjustPct, int startAdjustMS,
                                               const std::string& track) {
    std::map<int, std::vector<float>> res;
    std::vector<MidiNoteEvent> events;
    int trackCount = 0;
    if (!ParseMIDI(file, events, trackCount)) {
        spdlog::warn("Invalid MIDI file {}", file);
        return res;
    }

    int wantTrack = -1; // -1 = all
    if (track != "All" && !track.empty()) {
        wantTrack = (int)std::strtol(track.c_str(), nullptr, 10) - 1;
        if (wantTrack < 0 || wantTrack >= trackCount) wantTrack = -1;
    }

    float speedadjust = speedAdjustPct / 100.0f;

    // Walk the (already time-sorted per track) events, maintaining a
    // note on/off count per key; emit the active-note set at each
    // change boundary, like the desktop loader.
    // Merge events from selected track(s) and sort by time.
    std::vector<const MidiNoteEvent*> sel;
    for (const auto& e : events) {
        if (wantTrack < 0 || e.trackIndex == wantTrack) sel.push_back(&e);
    }
    std::stable_sort(sel.begin(), sel.end(), [](const MidiNoteEvent* a, const MidiNoteEvent* b) {
        return a->timeSeconds < b->timeSeconds;
    });

    int notestate[128] = { 0 };
    float lasttime = -1;
    for (const MidiNoteEvent* e : sel) {
        float time = (float)startAdjustMS / 100.0f + (float)e->timeSeconds * speedadjust;
        if (time != lasttime) {
            if (lasttime >= 0) {
                int start = RoundTS(lasttime, intervalMS);
                int end = RoundTS(time, intervalMS);
                for (int j = start; j < end; j += intervalMS) {
                    for (int k = 0; k <= 127; ++k) {
                        if (notestate[k] > 0) AddNoteAt(res, j, (float)k);
                    }
                }
            }
            lasttime = time;
        }
        int key = e->key;
        if (key < 0 || key > 127) continue;
        if (e->noteOn) {
            notestate[key]++;
        } else {
            if (notestate[key] > 0) notestate[key]--;
            int frame = RoundTS(time, intervalMS);
            if (frame >= 0) {
                // refresh the active set at this frame
                res.erase(frame);
                for (int k = 0; k <= 127; ++k) {
                    if (notestate[k] > 0) AddNoteAt(res, frame, (float)k);
                }
            }
        }
    }
    return res;
}

int MIDITrackCount(const std::string& file) {
    std::vector<MidiNoteEvent> events;
    int trackCount = 0;
    ParseMIDI(file, events, trackCount);
    return trackCount;
}

std::vector<NoteMark> BuildNoteMarks(const std::map<int, std::vector<float>>& notes,
                                     int intervalMS, int durationMS) {
    std::vector<NoteMark> marks;
    if (intervalMS <= 0) intervalMS = 1;
    int frames = durationMS / intervalMS;

    auto labelFor = [](const std::vector<float>& ns) {
        std::string s;
        for (float f : ns) {
            if (!s.empty()) s += ",";
            s += DecodeMidi((int)f);
        }
        return s;
    };

    int last = 0;
    std::string lastLabel;
    for (int i = 0; i <= frames; ++i) {
        std::string label;
        auto it = notes.find(i * intervalMS);
        if (it != notes.end()) label = labelFor(it->second);
        if (label == lastLabel) continue;
        if (i != 0) marks.push_back({ last * intervalMS, i * intervalMS, lastLabel });
        last = i;
        lastLabel = label;
    }
    marks.push_back({ last * intervalMS, frames * intervalMS, lastLabel });
    return marks;
}

} // namespace NoteImporter
