/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// I may still change the logic for choosing how to play each note. I am not convinced I have it right but i need feedback from someone using a real set of notes to give feedback on the choices.

#include <vector>

#include "../../include/guitar-16.xpm"
#include "../../include/guitar-64.xpm"

#include "GuitarEffect.h"
#include "GuitarPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsXmlFile.h"
#include "../UtilFunctions.h"
#include "models/Model.h"

#include <string>
#include <list>

#include <log4cpp/Category.hh>

class GuitarTiming
{
public:
    uint32_t _startMS = 0;
    uint32_t _endMS = 0;
    std::list<std::pair<uint8_t, uint8_t>> _fingerPos;
    GuitarTiming(uint32_t start, uint32_t end)
    {
        _startMS = start;
        _endMS = end;
    }

    void AddFingerPos(uint8_t string, uint8_t pos)
    {
        _fingerPos.push_back({ string, pos });
    }

    int GetPositionCentre() const
    {
        int zeroCount = 0;
        int total = 0;
        for (auto const& v : _fingerPos) {
            total += v.second;
            if (v.second == 0)
                ++zeroCount;
        }

        if (_fingerPos.size() - zeroCount == 0)
            return 0;

        return total / (_fingerPos.size() - zeroCount);
    }

    int GetSpread() const
    {
        int min = -1;
        int max = -1;

        for (auto const& v : _fingerPos) {
            // we ignore zero as it requires no fingers
            if (v.second != 0) {
                if (min == -1 || v.second < min)
                    min = v.second;
                if (max == -1 || v.second > max)
                    max = v.second;
            }
        }
        return max - min;
    }
};

// These are the string notes when no finger is on the string
typedef struct GuitarNotes
{
    uint8_t string;
    uint8_t fret;
    uint8_t note;
} GuitarNotes;

std::vector<GuitarNotes> bass = {
    { 0, 0, 28 }, // E1
    { 1, 0, 33 }, // A1
    { 2, 0, 38 }, // D2
    { 3, 0, 43 }, // G2
};

std::vector<GuitarNotes>
    guitar = {
        { 0, 0, 40 }, // E2
        { 1, 0, 45 }, // A3
        { 2, 0, 50 }, // D3
        { 3, 0, 55 }, // G3
        { 4, 0, 59 }, // B4
        { 5, 0, 64 }, // E4
    };

std::vector<GuitarNotes>
    banjo = {
        { 1, 0, 50 }, // D3
        { 2, 0, 55 }, // G3
        { 3, 0, 59 }, // B4
        { 4, 0, 62 }, // D4
        { 0, 0, 62 }, // D4
    };

std::vector<GuitarNotes>
    violin = {
        { 1, 0, 55 }, // G3
        { 2, 0, 62 }, // D4
        { 3, 0, 69 }, // A4
        { 4, 0, 76 }, // E5
    };

class Chordx
{
public:
    std ::string _name;
    std::list<uint8_t> _notes;
    std::list<std::pair<uint8_t, uint8_t>> _guitarTiming;
};

std::list<Chordx> __guitarChords = {
    // Major Chords
    { "CA", { 40, 45, 52, 57, 61, 64 }, { { 0, 0 }, { 1, 0 }, { 2, 2 }, { 3, 2 }, { 4, 2 }, { 5, 0 } } }, // A
    { "CA#", { 50, 58, 62, 65 }, { { 2, 0 }, { 3, 3 }, { 4, 3 }, { 5, 1 } } },                            // A# / Bb
    { "CBB", { 50, 58, 62, 65 }, { { 2, 0 }, { 3, 3 }, { 4, 3 }, { 5, 1 } } },                            // A# / Bb
    { "CB", { 54, 59, 63, 66 }, { { 2, 4 }, { 3, 4 }, { 4, 4 }, { 5, 2 } } },                             // B
    { "CC", { 40, 48, 52, 55, 60, 64 }, { { 0, 0 }, { 1, 3 }, { 2, 2 }, { 3, 0 }, { 4, 1 }, { 5, 0 } } }, // C
    { "CC#", { 53, 56, 61, 65 }, { { 2, 3 }, { 3, 1 }, { 4, 2 }, { 5, 1 } } },                            // C# / Db
    { "CDB", { 53, 56, 61, 65 }, { { 2, 3 }, { 3, 1 }, { 4, 2 }, { 5, 1 } } },                            // C# / Db
    { "CD", { 45, 50, 57, 62, 66 }, { { 1, 0 }, { 2, 0 }, { 3, 2 }, { 4, 3 }, { 5, 2 } } },               // D
    { "CD#", { 51, 58, 63, 67 }, { { 2, 1 }, { 3, 3 }, { 4, 4 }, { 5, 3 } } },                            // D# / Eb
    { "CEB", { 51, 58, 63, 67 }, { { 2, 1 }, { 3, 3 }, { 4, 4 }, { 5, 3 } } },                            // D# / Eb
    { "CE", { 40, 47, 52, 56, 59, 64 }, { { 0, 0 }, { 1, 2 }, { 2, 2 }, { 3, 1 }, { 4, 0 }, { 5, 0 } } }, // E
    { "CF", { 45, 53, 57, 60, 65 }, { { 1, 0 }, { 2, 3 }, { 3, 2 }, { 4, 1 }, { 5, 1 } } },               // F
    { "CF#", { 54, 58, 61, 66 }, { { 2, 4 }, { 3, 3 }, { 4, 2 }, { 5, 2 } } },                            // F# / Gb
    { "CGB", { 54, 58, 61, 66 }, { { 2, 4 }, { 3, 3 }, { 4, 2 }, { 5, 2 } } },                            // F# / Gb
    { "CG", { 43, 47, 50, 55, 59, 67 }, { { 0, 3 }, { 1, 2 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 3 } } }, // G
    { "CG#", { 51, 56, 60, 68 }, { { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 4 } } },                            // G# / Ab
    { "CAB", { 51, 56, 60, 68 }, { { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 4 } } },                            // G# / Ab
    // Minor Chords
    { "CAM", { 40, 45, 52, 57, 60, 64 }, { { 0, 0 }, { 1, 0 }, { 2, 2 }, { 3, 2 }, { 4, 1 }, { 5, 0 } } }, // Am
    { "CA#M", { 53, 58, 61, 65 }, { { 2, 3 }, { 3, 3 }, { 4, 2 }, { 5, 1 } } },                            // A#m / Bbm
    { "CBbM", { 53, 58, 61, 65 }, { { 2, 3 }, { 3, 3 }, { 4, 2 }, { 5, 1 } } },                            // A#m / Bbm
    { "CBM", { 54, 59, 62, 66 }, { { 2, 4 }, { 3, 4 }, { 4, 3 }, { 5, 2 } } },                             // Bm
    { "CCM", { 51, 55, 60, 67 }, { { 2, 1 }, { 3, 0 }, { 4, 1 }, { 5, 3 } } },                             // Cm
    { "CC#M", { 52, 56, 61, 64 }, { { 2, 2 }, { 3, 1 }, { 4, 2 }, { 5, 0 } } },                            // C#m / Dbm
    { "CDbM", { 52, 56, 61, 64 }, { { 2, 2 }, { 3, 1 }, { 4, 2 }, { 5, 0 } } },                            // C#m / Dbm
    { "CDM", { 45, 50, 57, 62, 65 }, { { 1, 0 }, { 2, 0 }, { 3, 2 }, { 4, 3 }, { 5, 1 } } },               // Dm
    { "CD#M", { 51, 58, 63, 66 }, { { 2, 1 }, { 3, 3 }, { 4, 4 }, { 5, 2 } } },                            // D#m / Ebm
    { "CEBM", { 51, 58, 63, 66 }, { { 2, 1 }, { 3, 3 }, { 4, 4 }, { 5, 2 } } },                            // D#m / Ebm
    { "CEM", { 40, 47, 52, 55, 59, 64 }, { { 0, 0 }, { 1, 2 }, { 2, 2 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } }, // Em
    { "CFM", { 53, 56, 60, 65 }, { { 2, 3 }, { 3, 1 }, { 4, 1 }, { 5, 1 } } },                             // Fm
    { "CF#M", { 54, 57, 61, 66 }, { { 2, 4 }, { 3, 2 }, { 4, 2 }, { 5, 2 } } },                            // F#m / Gbm
    { "CGBM", { 54, 57, 61, 66 }, { { 2, 4 }, { 3, 2 }, { 4, 2 }, { 5, 2 } } },                            // F#m / Gbm
    { "CGM", { 50, 58, 62, 67 }, { { 2, 0 }, { 3, 3 }, { 4, 3 }, { 5, 3 } } },                             // Gm
    { "CG#M", { 56, 59, 63, 68 }, { { 2, 6 }, { 3, 4 }, { 4, 4 }, { 5, 4 } } },                            // G#m / Abm
    { "CABM", { 56, 59, 63, 68 }, { { 2, 6 }, { 3, 4 }, { 4, 4 }, { 5, 4 } } },                            // G#m / Abm
    // Seventh Chords
    { "CA7", { 40, 45, 52, 55, 61, 64 }, { { 0, 0 }, { 1, 0 }, { 2, 2 }, { 3, 0 }, { 4, 2 }, { 5, 0 } } }, // A7
    { "CA#7", { 53, 58, 62, 68 }, { { 2, 3 }, { 3, 3 }, { 4, 3 }, { 5, 4 } } },                            // A#7 / Bb7
    { "CBB7", { 53, 58, 62, 68 }, { { 2, 3 }, { 3, 3 }, { 4, 3 }, { 5, 4 } } },                            // A#7 / Bb7
    { "CB7", { 47, 51, 57, 59, 66 }, { { 1, 2 }, { 2, 1 }, { 3, 2 }, { 4, 0 }, { 5, 2 } } },               // B7
    { "CC7", { 40, 48, 52, 58, 60, 64 }, { { 0, 0 }, { 1, 3 }, { 2, 2 }, { 3, 3 }, { 4, 1 }, { 5, 0 } } }, // C7
    { "CC#7", { 53, 56, 59, 65 }, { { 2, 3 }, { 3, 1 }, { 4, 0 }, { 5, 1 } } },                            // C#7 / Db7
    { "CDB7", { 53, 56, 59, 65 }, { { 2, 3 }, { 3, 1 }, { 4, 0 }, { 5, 1 } } },                            // C#7 / Db7
    { "CD7", { 45, 50, 57, 60, 66 }, { { 1, 0 }, { 2, 0 }, { 3, 2 }, { 4, 1 }, { 5, 2 } } },               // D7
    { "CD#7", { 51, 58, 61, 67 }, { { 2, 1 }, { 3, 3 }, { 4, 2 }, { 5, 3 } } },                            // D#7 / Eb7
    { "CEB7", { 51, 58, 61, 67 }, { { 2, 1 }, { 3, 3 }, { 4, 2 }, { 5, 3 } } },                            // D#7 / Eb7
    { "CE7", { 40, 47, 50, 56, 59, 64 }, { { 0, 0 }, { 1, 2 }, { 2, 0 }, { 3, 1 }, { 4, 0 }, { 5, 0 } } }, // E7
    { "CF7", { 45, 51, 57, 60, 65 }, { { 1, 0 }, { 2, 1 }, { 3, 2 }, { 4, 1 }, { 5, 1 } } },               // F7
    { "CF#7", { 54, 58, 61, 64 }, { { 2, 4 }, { 3, 3 }, { 4, 2 }, { 5, 0 } } },                            // F#7 / Gb7
    { "CGB7", { 54, 58, 61, 64 }, { { 2, 4 }, { 3, 3 }, { 4, 2 }, { 5, 0 } } },                            // F#7 / Gb7
    { "CG7", { 43, 47, 50, 55, 59, 65 }, { { 0, 3 }, { 1, 2 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 1 } } }, // G7
    { "CG#7", { 51, 56, 60, 66 }, { { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 2 } } },                            // G#7 / Ab7
    { "CAB7", { 51, 56, 60, 66 }, { { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 2 } } },                            // G#7 / Ab7
};

std::list<Chordx> __bassChords = {
    // Major Chords
    { "CA", { 33, 37, 40, 45 }, { { 0, 5 }, { 1, 4 }, { 2, 2 }, { 3, 3 } } }, // A
    { "CB", { 30, 35, 39, 47 }, { { 0, 2 }, { 1, 2 }, { 2, 1 }, { 3, 4 } } }, // B
    { "CC", { 31, 36, 40, 48 }, { { 0, 3 }, { 1, 3 }, { 2, 2 }, { 3, 5 } } }, // C
    { "CD", { 30, 38, 39, 45 }, { { 0, 2 }, { 1, 5 }, { 2, 0 }, { 3, 2 } } }, // D
    { "CE", { 28, 35, 40, 44 }, { { 0, 0 }, { 1, 2 }, { 2, 2 }, { 3, 1 } } }, // E
    { "CF", { 29, 36, 41, 45 }, { { 0, 1 }, { 1, 3 }, { 2, 3 }, { 3, 2 } } }, // F
    { "CG", { 31, 35, 38, 43 }, { { 0, 3 }, { 1, 2 }, { 2, 0 }, { 3, 0 } } }, // G
    // Minor Chords
    { "CAM", { 33, 36, 40, 45 }, { { 0, 5 }, { 1, 3 }, { 2, 2 }, { 3, 3 } } }, // Am
    { "CBM", { 30, 35, 38, 47 }, { { 0, 2 }, { 1, 2 }, { 2, 0 }, { 3, 4 } } }, // Bm
    { "CCM", { 31, 36, 39, 48 }, { { 0, 3 }, { 1, 3 }, { 2, 1 }, { 3, 5 } } }, // Cm
    { "CDM", { 29, 38, 39, 45 }, { { 0, 1 }, { 1, 5 }, { 2, 0 }, { 3, 2 } } }, // Dm
    { "CEM", { 28, 35, 40, 43 }, { { 0, 0 }, { 1, 2 }, { 2, 2 }, { 3, 0 } } }, // Em
    { "CFM", { 29, 36, 41, 44 }, { { 0, 1 }, { 1, 3 }, { 2, 3 }, { 3, 1 } } }, // Fm
    { "CGM", { 31, 34, 38, 43 }, { { 0, 3 }, { 1, 1 }, { 2, 0 }, { 3, 0 } } }, // Gm
    // Seventh Chords
    { "CA7", { 31, 37, 40, 45 }, { { 0, 3 }, { 1, 4 }, { 2, 2 }, { 3, 3 } } }, // A7
    { "CB7", { 30, 35, 39, 45 }, { { 0, 2 }, { 1, 2 }, { 2, 1 }, { 3, 2 } } }, // B7
    { "CC7", { 31, 36, 40, 46 }, { { 0, 3 }, { 1, 3 }, { 2, 2 }, { 3, 3 } } }, // C7
    { "CD7", { 30, 36, 39, 45 }, { { 0, 2 }, { 1, 3 }, { 2, 0 }, { 3, 2 } } }, // D7
    { "CE7", { 28, 35, 38, 44 }, { { 0, 0 }, { 1, 2 }, { 2, 0 }, { 3, 1 } } }, // E7
    { "CF7", { 29, 36, 39, 45 }, { { 0, 1 }, { 1, 3 }, { 2, 1 }, { 3, 2 } } }, // F7
    { "CG7", { 29, 35, 38, 43 }, { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 3, 0 } } }, // G7
};

std::list<Chordx> __banjoChords = {
    // Major Chords
    { "CA", { 52, 57, 61, 64 }, { { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 2 } } }, // A
    { "CA#", { 53, 58, 62, 65 }, { { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 } } }, // A#
    { "CBB", { 53, 58, 62, 65 }, { { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 } } }, // Bb
    { "CD", { 50, 57, 62, 66 }, { { 1, 0 }, { 2, 2 }, { 3, 3 }, { 4, 4 } } },  // D
    { "CE", { 52, 56, 59, 64 }, { { 1, 2 }, { 2, 1 }, { 3, 0 }, { 4, 2 } } },  // E
    { "CF", { 53, 57, 60, 65 }, { { 1, 3 }, { 2, 2 }, { 3, 1 }, { 4, 3 } } },  // F
    { "CG", { 50, 55, 59, 62 }, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 } } },  // G
    // Minor Chords
    { "CAM", { 52, 57, 60, 64 }, { { 1, 2 }, { 2, 2 }, { 3, 1 }, { 4, 2 } } }, // Am
    { "CDM", { 53, 57, 62, 65 }, { { 1, 3 }, { 2, 2 }, { 3, 3 }, { 4, 3 } } },  // Dm
    { "CEM", { 52, 55, 59, 64 }, { { 1, 2 }, { 2, 0 }, { 3, 0 }, { 4, 2 } } },  // Em
    // Seventh Chords
    { "CD7", { 50, 57, 60, 62 }, { { 1, 0 }, { 2, 2 }, { 3, 1 }, { 4, 0 } } }, // D7
    { "CG7", { 50, 55, 59, 65 }, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 3 } } },  // G7
};

std::list<Chordx> __violinChords = {};

bool centresort(const GuitarTiming* first, const GuitarTiming* second)
{
    return first->GetPositionCentre() < second->GetPositionCentre();
}

class NoteTiming
{
    // zero is an open string. 1 is longest and deepest, maxFrets is shortest and highest
    int GetFretPos(uint8_t string, uint8_t note, const std::string& type, uint8_t maxFrets)
    {
        auto base = NoteTiming::GetBaseNotes(type);
        if (base == nullptr)
            return -1;

        // check we have enough strings
        if (string >= base->size())
            return -1;

        // too low for this string
        if (note < base->at(string).note)
            return -1;

        // too high for this string
        if (note > base->at(string).note + maxFrets)
            return -1;

        return note - base->at(string).note;
    }

public:
    uint32_t _startMS = 0;
    uint32_t _endMS = 0;
    std::list<uint8_t> _notes;
    std::list<GuitarTiming*> _possibleTimings;

    NoteTiming(const NoteTiming& nt)
    {
        _startMS = nt._startMS;
        _endMS = nt._endMS;
        _notes = nt._notes;
        for (auto it : nt._possibleTimings) {
            _possibleTimings.push_back(new GuitarTiming(*it));
        }
    }

    void SetGuitarTiming(GuitarTiming* gt)
    {
        ClearPossibleTimings();
        _possibleTimings.push_back(new GuitarTiming(*gt));
    }

    static const std::vector<GuitarNotes>* GetBaseNotes(const std::string& type)
    {
        if (type == "Guitar")
            return &guitar;
        else if (type == "Bass Guitar")
            return &bass;
        else if (type == "Banjo")
            return &banjo;
        else
            return &violin;
    }

    static size_t GetStrings(const std::string& type)
    {
        auto base = GetBaseNotes(type);
        if (base == nullptr)
            return 0;

        return base->size();
    }

    NoteTiming(uint32_t start, uint32_t end)
    {
        _startMS = start;
        _endMS = end;
    }

    NoteTiming(uint32_t start, uint32_t end, std::list<uint8_t> notes)
    {
        _startMS = start;
        _endMS = end;
        _notes = notes;
    }

    ~NoteTiming()
    {
        ClearPossibleTimings();
    }

    void ClearPossibleTimings()
    {
        while (_possibleTimings.size() > 0) {
            delete _possibleTimings.front();
            _possibleTimings.pop_front();
        }
    }

    int PickTimingClosestTo(int centre, int& lastString)
    {
        int ls = lastString;
        lastString = -1;

        if (_possibleTimings.size() == 0) {
            return -1;
        }

        if (_possibleTimings.size() == 1)
            return _possibleTimings.front()->GetPositionCentre();

        auto closest = _possibleTimings.begin();
        auto it = closest;
        auto closestOnDifferentString = closest;
        int diffOnDifferentString = 9999;
        bool different = false;
        int diff = std::abs((*closest)->GetPositionCentre() - centre);

        while (it != _possibleTimings.end()) {
            if (std::abs((*it)->GetPositionCentre() - centre) < diff) {
                closest = it;
                diff = std::abs((*it)->GetPositionCentre() - centre);
            }

            // if we were playing a single note and now we are playing a single note then try to use a different string
            if (std::abs((*it)->GetPositionCentre() - centre) < diffOnDifferentString && ls != -1 && (*it)->_fingerPos.size() == 1 && (*it)->_fingerPos.front().first != ls) {
                closestOnDifferentString = it;
                diffOnDifferentString = std::abs((*it)->GetPositionCentre() - centre);
                different = true;
            } else if (closestOnDifferentString == it && ls != -1 && (*it)->_fingerPos.size() == 1 && (*it)->_fingerPos.front().first != ls) {
                different = true;
            }

            ++it;
        }

        // Use this to check the closest on different string code
        ///if (different) {
        //    if ((*closestOnDifferentString)->_fingerPos.size() == 1 && ls == (*closestOnDifferentString)->_fingerPos.front().first) {
        //        int a = 0;
        //    }
        //} else {
        //    if ((*closest)->_fingerPos.size() == 1 && ls == (*closest)->_fingerPos.front().first) {
        //        int a = 0;
        //    }
        //}

        // Remove all timings but our selected one
        if (different) {
            while (_possibleTimings.begin() != closestOnDifferentString) {
                delete _possibleTimings.front();
                _possibleTimings.pop_front();
            }
        } else {
            while (_possibleTimings.begin() != closest) {
                delete _possibleTimings.front();
                _possibleTimings.pop_front();
            }
        }
        while (_possibleTimings.size() != 1) {
            auto todel = _possibleTimings.begin();
            ++todel;
            delete *todel;
            _possibleTimings.erase(todel);
        }

        if (_possibleTimings.front()->_fingerPos.size() == 1) {
            lastString = _possibleTimings.front()->_fingerPos.front().first;
        }

        return _possibleTimings.front()->GetPositionCentre();
    }

    
    bool listsMatch(const std::list<uint8_t>& list1, const std::list<uint8_t>& list2)
    {
        // Check if both lists have the same size
        if (list1.size() != list2.size())
            return false;

        // Iterate through each element in list1
        for (auto it = list1.begin(); it != list1.end(); ++it) {
            // Check if the current element exists in list2
            if (std::find(list2.begin(), list2.end(), *it) == list2.end())
                return false;
        }

        // All elements match
        return true;
    }

    bool CheckChords(const std::string& type)
    {
        auto chords = __guitarChords;

        if (type == "Bass")
            chords = __bassChords;
        else if (type == "Banjo")
            chords = __banjoChords;
        else if (type == "Violin")
            chords = __violinChords;

        for (auto& it : chords) {
            if (it._notes.size() == _notes.size()) {
                if (listsMatch(it._notes, _notes)) {
                    auto t = new GuitarTiming(_startMS, _endMS);
                    for (auto const& it2 : it._guitarTiming) {
                        t->AddFingerPos(it2.first, it2.second);
                    }
                    _possibleTimings.push_back(t);
                    return true;
                }
            }
        }

        return false;
    }

    void GeneratePossibleTimings(const std::string& type, uint8_t maxFrets)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        // if there are no notes it may be because the timings were set using SnnPnn
        if (_notes.size() == 0)
            return;

        ClearPossibleTimings();

        _notes.sort();

        // check if our notes are one of the well known chords and if so use the defined fingering
        if (CheckChords(type))
            return;

        uint8_t strings = NoteTiming::GetStrings(type);

        std::list<uint8_t> firstNoteString;

        auto it = _notes.begin();
        while (it != _notes.end() && firstNoteString.size() == 0) {
            for (uint8_t i = 0; i < strings; ++i) {
                if (GetFretPos(i, *it, type, maxFrets) != -1) {
                    firstNoteString.push_back(i);
                }
            }
            ++it;
        }

        for (auto f : firstNoteString) {
            for (uint8_t j = f; j < strings; ++j) {
#ifdef FILTER_OUT_UPLAYABLE
                bool ok = true;
#endif
                auto fns = j;
                GuitarTiming* t = new GuitarTiming(_startMS, _endMS);
                for (auto n : _notes) {
#ifdef FILTER_OUT_UPLAYABLE
                    bool found = false;
#endif
                    for (uint8_t s = fns; s < strings; ++s) {
                        int fp = GetFretPos(s, n, type, maxFrets);
                        if (fp != -1) {
                            t->AddFingerPos(s, fp);
#ifdef FILTER_OUT_UPLAYABLE
                            found = true;
#endif
                            fns = s+1;
                            break;
                        }
                    }

#ifdef FILTER_OUT_UPLAYABLE
                    if (!found)
                        ok = false;
#endif
                }

                if (t->_fingerPos.size() == 0) {
                    delete t;
                } else {
#ifdef FILTER_OUT_UPLAYABLE
                    if (!ok)
                        delete t;
                    else
#endif
                        _possibleTimings.push_back(t);
                }
            }
        }

        bool allnotes = false;
        uint8_t max = 0;
        for (auto& it : _possibleTimings)
        {
            if (it->_fingerPos.size() == _notes.size())
                allnotes = true;
            if (it->_fingerPos.size() > max)
                max = it->_fingerPos.size();
        }

        if (allnotes)
        {
            // remove any possible timings not including all notes
            auto it = _possibleTimings.begin();
            while (it != _possibleTimings.end())
            {
                if ((*it)->_fingerPos.size() != _notes.size())
                {
                    it = _possibleTimings.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        else
        {
            // remove any possible timings not including the maximum notes
            auto it = _possibleTimings.begin();
            while (it != _possibleTimings.end()) {
                if ((*it)->_fingerPos.size() != max) {
                    it = _possibleTimings.erase(it);
                } else {
                    ++it;
                }
            }
        }

        if (max != _notes.size())
        {
            logger_base.warn("One or more notes not found on %s at %lu.", (const char*)type.c_str(), _startMS);
        }

        bool allZero = false;

        // prioritise centres that are lower (hence longer)
        _possibleTimings.sort(centresort);

        // remove the largest finger spreads until no more than 3 are left
        while (_possibleTimings.size() > 3 && !allZero) {
            allZero = true;

            auto max = _possibleTimings.begin();
            int maxSpread = (*max)->GetSpread();
            auto it = max;

            while (it != _possibleTimings.end()) {
                if ((*it)->GetSpread() != 0)
                    allZero = false;
                if ((*it)->GetSpread() >= maxSpread)
                    max = it;
                ++it;
            }

            // we only erase the maximum if they are not all zero
            if (!allZero)
                _possibleTimings.erase(max);
        }
    }
};

GuitarEffect::GuitarEffect(int id) :
    RenderableEffect(id, "Guitar", Guitar_16_xpm, Guitar_64_xpm, Guitar_64_xpm, Guitar_64_xpm, Guitar_64_xpm) 
{
    //ctor
	_panel = nullptr;
}

GuitarEffect::~GuitarEffect()
{
    //dtor
}

std::list<std::string> GuitarEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (settings.Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", "") == "")
    {
        res.push_back(wxString::Format("    ERR: Guitar effect needs a timing track. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else
    {
        std::list<NoteTiming*> timings = LoadTimingTrack(settings.Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", ""), 50, "Guitar", 100, 6);
        if (timings.size() == 0)
        {
            res.push_back(wxString::Format("    ERR: Guitar effect timing track '%s' has no notes. Model '%s', Start %s", settings.Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", ""), model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        while (timings.size() > 0)
        {
            delete timings.front();
            timings.pop_front();
        }
    }

    return res;
}

void GuitarEffect::SetPanelStatus(Model *cls)
{
    SetPanelTimingTracks();
}

void GuitarEffect::SetPanelTimingTracks()
{
    GuitarPanel *fp = (GuitarPanel*)panel;
    if (fp == nullptr)
    {
        return;
    }

    if (mSequenceElements == nullptr)
    {
        return;
    }

    // Load the names of the timing tracks
    std::string timingtracks = GetTimingTracks(1);
    wxCommandEvent event(EVT_SETTIMINGTRACKS);
    event.SetString(timingtracks);
    wxPostEvent(fp, event);
}

xlEffectPanel *GuitarEffect::CreatePanel(wxWindow *parent) {
    _panel = new GuitarPanel(parent);
	return _panel;
}

void GuitarEffect::SetDefaultParameters() {
    GuitarPanel *pp = (GuitarPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    SetChoiceValue(pp->Choice_Guitar_Type, "Guitar");
    SetChoiceValue(pp->Choice_StringAppearance, "On");
    SetSliderValue(pp->Slider_MaxFrets, 19);
    SetCheckBoxValue(pp->CheckBox_Collapse, false);
    SetCheckBoxValue(pp->CheckBox_Fade, false);
    SetCheckBoxValue(pp->CheckBox_ShowStrings, false);
    SetCheckBoxValue(pp->CheckBox_VaryWaveLengthOnFret, true);
    SetSliderValue(pp->Slider_StringWaveFactor, 0);
    SetSliderValue(pp->Slider_BaseWaveFactor, 10);
    SetPanelTimingTracks();
}

void GuitarEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Guitar_MIDITrack_APPLYLAST"] = wxString(newname);
    }

    SetPanelTimingTracks();
}

void GuitarEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    RenderGuitar(buffer,
                 effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                 std::string(SettingsMap.Get("CHOICE_Guitar_Type", "Guitar")),
                 std::string(SettingsMap.Get("CHOICE_Guitar_MIDITrack_APPLYLAST", "")),
                 std::string(SettingsMap.Get("CHOICE_StringAppearance", "On")),
                 SettingsMap.GetInt("SLIDER_MaxFrets", 19),
                 SettingsMap.GetBool("CHECKBOX_ShowStrings", false),
                 SettingsMap.GetBool("CHECKBOX_Fade", false),
                 SettingsMap.GetBool("CHECKBOX_Collapse", false),
                 SettingsMap.GetFloat("SLIDER_StringWaveFactor", 0.0) / 10.0,
                 SettingsMap.GetFloat("SLIDER_BaseWaveFactor", 1.0) / 10.0,
                 SettingsMap.GetBool("CHECKBOX_VaryWaveLengthOnFret", false));
}

bool notesort(const NoteTiming* first, const NoteTiming* second)
{
    return first->_startMS < second->_startMS;
}

class GuitarCache : public EffectRenderCache
{
public:
	GuitarCache() { };
	virtual ~GuitarCache() { 
        ClearTimings();
    };

    void ClearTimings()
    {
        while (_notes.size() > 0) {
            delete _notes.front();
            _notes.pop_front();
        }
    }

    void SetTimings(std::list<NoteTiming*>& notes, const std::string& type, uint8_t maxFrets)
    {
        ClearTimings();
        for (auto& it : notes)
        {
            _notes.push_back(new NoteTiming(*it));
        }
        ConvertToGuitarTiming(type, maxFrets);
    }

    void ConvertToGuitarTiming(const std::string& type, uint8_t maxFrets)
    {
        // sort the notes
        _notes.sort(notesort);

        // go through each one generating possible finger positions ... up to a maximum of 3 prioritising smallest finger spreads
        for (auto& it : _notes) {
            it->GeneratePossibleTimings(type, maxFrets);
        }

        // choose the one that follows the prior with the least movement
        int centre = -1;
        int lastString = -1;
        uint32_t lastEnd = 0;
        for (auto& it : _notes) {

            // if there is a gap in the notes then using the same string again is ok
            if (it->_startMS > lastEnd)
            {
                lastString = -1;
            }

            // if there is no centre favour the longest string possible
            if (centre == -1)
                centre = 0;

            // pick the next hand location from the available options
            int nc = it->PickTimingClosestTo(centre, lastString);

            // Dont change the centre if we are playing a full string as that does not need a hand movement
            if (nc != 0)
                centre = nc;

            lastEnd = it->_endMS;
        }
    }

    GuitarTiming* GetTimingAt(uint32_t ms)
    {
        for (const auto& it : _notes)
        {
            if (it->_startMS <= ms && it->_endMS > ms)
            {
                if (it->_possibleTimings.size() > 0)
                    return it->_possibleTimings.front();
                else
                    return nullptr;
            }
        }

        return nullptr;
    }

    std::list<NoteTiming*> _notes;
    std::string _MIDItrack;
};

//render Guitar fx during sequence:
void GuitarEffect::RenderGuitar(RenderBuffer& buffer, SequenceElements* elements, const std::string& type, const std::string& MIDITrack, const std::string& stringAppearance, int maxFrets, bool showStrings, bool fade, bool collapse, double stringWaveFactor, double baseWaveFactor, bool varyWavelengthBasedOnFret)
{
    GuitarCache* cache = (GuitarCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new GuitarCache();
        buffer.infoCache[id] = cache;
    }

    std::string& _MIDITrack = cache->_MIDItrack;

    uint8_t strings = NoteTiming::GetStrings(type);

    if (buffer.needToInit) {
        // just in case the timing tracks have changed
        SetPanelTimingTracks();

        buffer.needToInit = false;
        if (_MIDITrack != MIDITrack) {
            cache->ClearTimings();
            auto notes = LoadTimingTrack(MIDITrack, buffer.frameTimeInMs, type, maxFrets, strings);
            cache->SetTimings(notes, type, maxFrets);
            elements->AddRenderDependency(MIDITrack, buffer.cur_model);

            while (notes.size() > 0)
            {
                delete notes.front();
                notes.pop_front();
            }
        }

        _MIDITrack = MIDITrack;

        if (_MIDITrack != "") {
            elements->AddRenderDependency(_MIDITrack, buffer.cur_model);
        }
    }

    uint32_t time = buffer.curPeriod * buffer.frameTimeInMs;

    DrawGuitar(buffer, cache->GetTimingAt(time), stringAppearance, maxFrets, strings, showStrings, fade, collapse, stringWaveFactor, baseWaveFactor, varyWavelengthBasedOnFret);
}

inline uint32_t FlipY(uint32_t y, uint32_t height)
{
    return height - y - 1;
}

void GuitarEffect::DrawGuitarOn(RenderBuffer& buffer, uint8_t string, uint8_t fretPos, uint32_t timePos, uint32_t of, uint8_t maxFrets, uint8_t strings, bool showStrings, bool fade, bool collapse)
{
    xlColor c;
    buffer.palette.GetColor(string % buffer.palette.Size(), c);

    xlColor cc = c;

    float alpha = (float)(of - timePos) / (float)of;
    if (fade)
        c.alpha = 255.0 * alpha;

    float perString = (float)buffer.BufferHt / strings;

    uint32_t maxX = ((maxFrets - fretPos) * buffer.BufferWi) / maxFrets;
    if (showStrings) {
        for (uint32_t x = maxX; x < buffer.BufferWi; ++x) {
            buffer.SetPixel(x, FlipY(perString * string + perString / 2, buffer.BufferHt), cc);
        }
    }

    int centre = perString * string + perString / 2;
    int height = perString;
    if (collapse) {
        height *= alpha;
        // always draw the string so dont let the height get to zero
        if (height < 1)
            height = 1;
    }

    int start = centre - height / 2;

    for (uint32_t x = 0; x < maxX; ++x) {
        for (uint32_t y = start; y < start + height; ++y) {
            buffer.SetPixel(x, FlipY(y, buffer.BufferHt), c);
        }
    }
}

#define WAVE_RAMP 3.0

void GuitarEffect::DrawGuitarWave(RenderBuffer& buffer, uint8_t string, uint8_t fretPos, uint32_t timePos, uint32_t of, uint8_t maxFrets, uint8_t strings, bool showStrings, bool fade, bool collapse, double stringWaveFactor, double baseWaveFactor, bool varyWavelengthBasedOnFret)
{
    xlColor c;
    buffer.palette.GetColor(string % buffer.palette.Size(), c);

    xlColor cc = c;

    float alpha = (float)(of - timePos) / (float)of;
    if (fade)
        c.alpha = 255.0 * alpha;

    uint32_t cycles = (((maxFrets - fretPos) * buffer.BufferWi) / maxFrets) / 10;
    double perString = (float)buffer.BufferHt / strings;
    double maxX = ((maxFrets - fretPos) * buffer.BufferWi) / maxFrets;

    if (showStrings) {
        for (uint32_t x = maxX; x < buffer.BufferWi; ++x) {
            buffer.SetPixel(x, FlipY(perString * string + perString / 2, buffer.BufferHt), cc);
        }
    }

    double diffPerFret = 0.0;
    if (varyWavelengthBasedOnFret)
    {
        diffPerFret = 0.3;
    }

    for (uint32_t x = 0; x < maxX; ++x) {

        // this foces the wave to zero near the ends
        double maxY = perString;

        if (collapse)
            maxY *= alpha;

        if (x < WAVE_RAMP) {
            maxY *= ((double)x * 1.0 / WAVE_RAMP);
        } else if (x >= maxX - WAVE_RAMP - 1) {
            maxY *= (double)(maxX - x - 1) * 1.0 / WAVE_RAMP;
        }

        #define WAVE_SPEED 2
        uint32_t y = (maxY / 2.0) * sin((PI * 2.0 * cycles * (double)x / (((double)(strings - string - 1) * stringWaveFactor) + baseWaveFactor + (maxFrets - fretPos) * diffPerFret)) / maxX + (timePos * WAVE_SPEED));
        y += (perString / 2.0) + (perString * string);
        buffer.SetPixel(x, FlipY(y, buffer.BufferHt), c);
    }
}

void GuitarEffect::DrawString(RenderBuffer& buffer, uint8_t string, uint8_t strings)
{
    xlColor c;
    buffer.palette.GetColor(string, c);

    double perString = (float)buffer.BufferHt / strings;
    for (uint32_t x = 0; x < buffer.BufferWi; ++x) {
        buffer.SetPixel(x, FlipY(perString * string + perString / 2, buffer.BufferHt), c);
    }
}

void GuitarEffect::DrawGuitar(RenderBuffer& buffer, GuitarTiming* pdata, const std::string& stringAppearance, uint8_t maxFrets, uint8_t strings, bool showStrings, bool fade, bool collapse, double stringWaveFactor, double baseWaveFactor, bool varyWavelengthBasedOnFret)
{
    if (pdata != nullptr) {
        uint32_t pos = (buffer.curPeriod * buffer.frameTimeInMs - pdata->_startMS) / buffer.frameTimeInMs;
        uint32_t len = (pdata->_endMS - pdata->_startMS) / buffer.frameTimeInMs;

        if (stringAppearance == "On") {
            for (const auto& it : pdata->_fingerPos) {
                DrawGuitarOn(buffer, it.first, it.second, pos, len, maxFrets, strings, showStrings, fade, collapse);
            }
        } else if (stringAppearance == "Wave") {
            for (const auto& it : pdata->_fingerPos) {
                DrawGuitarWave(buffer, it.first, it.second, pos, len, maxFrets, strings, showStrings, fade, collapse, stringWaveFactor, baseWaveFactor, varyWavelengthBasedOnFret);
            }
        }
    }

    if (showStrings) {
        for (uint8_t s = 0; s < strings; ++s) {
            bool found = false;
            if (pdata != nullptr) {
                for (const auto& it : pdata->_fingerPos) {
                    if (it.first == s) {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                DrawString(buffer, s, strings);
        }
    }
}

std::vector<float> GuitarEffect::Parse(wxString& l)
{
	std::vector<float> res;
	wxString s = l;
	while (s.Len() != 0)
	{
		int end = s.First('\t');
		if (end > 0)
		{
			res.push_back(wxAtof(s.SubString(0, end - 1)));
			s = s.Right(s.Len() - end - 1);
		}
		else
		{
			res.push_back(wxAtof(s));
			s = "";
		}
	}

	return res;
}

std::list<std::string> GuitarEffect::ExtractNotes(const std::string& label)
{
    std::string n = label;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    std::list<std::string> res;

    std::string s = "";
    for (const auto& it : n)
    {
        if (it == ':' || it == ' ' || it == ';' || it == ',')
        {
            if (s != "")
            {
                res.push_back(s);
                s = "";
            }
        }
        else
        {
            if ((it >= 'A' && it <= 'G') ||
                (it == '#') ||
                (it == 'S') ||
                (it == 'P') ||
                (it == 'M') ||
                (it >= '0' && it <= '9'))
            {
                s += it;
            }
        }
    }

    if (s != "")
    {
        res.push_back(s);
    }

    return res;
}

void GuitarEffect::ConvertStringPos(const std::string& note, uint8_t& string, uint8_t& pos)
{
    string = 0xFF;
    pos = 0xFF;

    std::string n = note;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    if (n[0] != 'S')
        return;

    string = 0;
    uint32_t index = 1;

    while (n[index] >= '0' && n[index] <= '9')
    {
        string *= 10;
        string += (uint8_t)(n[index] - '0');
        ++index;
    }

    if (n[index++] != 'P') {
        string = 0xFF;
        return;
    }

    pos = 0;
    while (n[index] >= '0' && n[index] <= '9') {
        pos *= 10;
        pos += (uint8_t)(n[index] - '0');
        ++index;
    }
}

int GuitarEffect::ConvertNote(const std::string& note)
{
    std::string n = note;
    int nletter;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    switch (n[0])
    {
    case 'S':
    case 'P':
        return -1;
    case 'A':
        nletter = 9;
        break;
    case 'B':
        nletter = 11;
        break;
    case 'C':
        nletter = 0;
        break;
    case 'D':
        nletter = 2;
        break;
    case 'E':
        nletter = 4;
        break;
    case 'F':
        nletter = 5;
        break;
    case 'G':
        nletter = 7;
        break;
    default:
        {
            int number = wxAtoi(n);
            if (number < 0) number = 0;
            if (number > 127) number = 127;
            return number;
        }
    }

    n = n.substr(1);
    int sharp = 0;
    if (n.find('#') != std::string::npos)
    {
        sharp = 1;
    }
    else if (n.find('B') != std::string::npos)
    {
        sharp = -1;
    }

    int octave = 4;

    if (n != "")
    {
        if (n[0] == '#')
        {
            n = n.substr(1);
        }
        else if (n[0] == 'B')
        {
            n = n.substr(1);
        }
    }

    if (n != "")
    {
        octave = wxAtoi(n);
    }

    int number = 12 + (octave * 12) + nletter + sharp;
    if (number < 0) number = 0;
    if (number > 127) number = 127;
    return number;
}

std::list<NoteTiming*> GuitarEffect::LoadTimingTrack(const std::string& track, int intervalMS, const std::string& type, uint8_t maxFrets, uint8_t strings)
{
    static log4cpp::Category& logger_Guitardata = log4cpp::Category::getInstance(std::string("log_Guitardata"));

    std::list<NoteTiming*> res;

    logger_Guitardata.debug("Loading timings from timing track " + track);

    if (mSequenceElements == nullptr) {
        logger_Guitardata.debug("No timing tracks found.");
        return res;
    }

    // Load the names of the timing tracks
    EffectLayer* el = GetTiming(track);

    if (el == nullptr) {
        logger_Guitardata.debug("Timing track not found.");
        return res;
    }

    for (int j = 0; j < el->GetEffectCount(); ++j) {
        std::list<uint8_t> notes;
        int starttime = el->GetEffect(j)->GetStartTimeMS();
        int endtime = el->GetEffect(j)->GetEndTimeMS();
        GuitarTiming t(starttime, endtime);
        std::string label = el->GetEffect(j)->GetEffectName();
        std::list<std::string> notelabels = ExtractNotes(label);
        for (const auto& s : notelabels) {
            bool chord = false;
            auto chords = __guitarChords;

            if (type == "Bass")
                chords = __bassChords;
            else if (type == "Banjo")
                chords = __banjoChords;
            else if (type == "Violin")
                chords = __violinChords;

            // If the label matches one of our chord names then use the chord notes
            std::string n = s;
            std::transform(n.begin(), n.end(), n.begin(), ::toupper);
            for (const auto& c : chords) {
                if (n == c._name) {
                    for (auto nn : c._notes) {
                        notes.push_back(nn);
                    }
                    chord = true;
                }
            }

            if (!chord) {
                uint8_t n = ConvertNote(s);
                if (n != 0xFF) {
                    notes.push_back(n);
                } else {
                    uint8_t string;
                    uint8_t pos;
                    ConvertStringPos(s, string, pos);
                    if (string != 0xFF && string != 0 && pos != 0xFF && pos <= maxFrets) {
                        t.AddFingerPos(strings - (string - 1) - 1, pos);
                    }
                }
            }
        }
        if (notes.size() != 0)
            res.push_back(new NoteTiming(starttime, endtime, notes));
        else if (t._fingerPos.size() > 0) {
            NoteTiming* nt = new NoteTiming(starttime, endtime);
            nt->SetGuitarTiming(&t);
            res.push_back(nt);
        }
    }

    return res;
}
