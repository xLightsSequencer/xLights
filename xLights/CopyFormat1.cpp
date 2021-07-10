/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CopyFormat1.h"
#include "sequencer/Effect.h"

#include <wx/wx.h>
#include "sequencer/EffectLayer.h"

CopyFormat1::~CopyFormat1()
{
    while (_effects.size() > 0) {
        delete _effects.front();
        _effects.pop_front();
    }
    while (_timings.size() > 0) {
        delete _timings.front();
        _timings.pop_front();
    }
}

CopyFormat1::CopyFormat1()
{
    _ok = true;
    _numTimingRows = 0;
    _lastRow = 0;
    _startTime = -1000;
    _startColumn = -1;
    _pasteByCell = false;
}

CopyFormat1::CopyFormat1(const std::string& data)
{
    _ok = false;

    _startTime = -1000;

    wxArrayString lines = wxSplit(data, '\n');

    if (lines.size() < 2) {
        // copyformat1 needs at least one header and one data row
        return;
    }

    // parse the header line
    wxArrayString cols = wxSplit(lines[0], '\t');

    if (cols.size() != 7) {
        // not the right number of columns in the header
        return;
    }

    if (cols[0] != "CopyFormat1") {
        // not the right format
        return;
    }

    int numTimings = wxAtoi(cols[1]);
    //int numEffects = wxAtoi(cols[2]);
    _numTimingRows = wxAtoi(cols[3]);
    _lastRow = wxAtoi(cols[4]);
    _startColumn = wxAtoi(cols[5]);
    _pasteByCell = false;
    if (cols[6] == "PASTE_BY_CELL") {
        _pasteByCell = true;
    }

    int firstRow = 999999;
    int lastRow = -1;
    for (int i = 1; i < lines.size(); ++i) {
        if (lines[i] != "") {
            if (i <= numTimings) {
                CopyFormat1Timing* timing = new CopyFormat1Timing(lines[i].ToStdString());
                _timings.push_back(timing);
                if (!timing->IsOk()) {
                    // fail because an effect line was invalid
                    return;
                }
            }
            else {
                CopyFormat1Effect* effect = new CopyFormat1Effect(lines[i].ToStdString(), _pasteByCell);
                _effects.push_back(effect);
                if (!effect->IsOk()) {
                    // fail because an effect line was invalid
                    return;
                }

                if (effect->Row() < firstRow) firstRow = effect->Row();
                if (effect->Row() > lastRow) lastRow = effect->Row();
            }
        }
    }

    _ok = true;
}

std::string CopyFormat1::Serialise() const
{
    wxString data = wxString::Format("CopyFormat1\t%d\t%d\t%d\t%d\t%d\t%s",
        (int)_timings.size(),
        (int)_effects.size(),
        _numTimingRows,
        CalcTimingRows(),
        _startColumn,
        (_pasteByCell && _startColumn >= 0 ? "PASTE_BY_CELL" : "NO_PASTE_BY_CELL"));

    for (auto it = _timings.begin(); it != _timings.end(); ++it) {
        data += "\n" + (*it)->Serialise();
    }

    for (auto it = _effects.begin(); it != _effects.end(); ++it) {
        data += "\n" + (*it)->Serialise();
    }

    data += "\n";

    return data.ToStdString();
}

int CopyFormat1::CalcTimingRows() const
{
    int minRow = 99999;
    int maxRow = -1;

    for (auto it = _timings.begin(); it != _timings.end(); ++it) {
        int row = (*it)->Row();
        if (row < minRow) minRow = row;
        if (row > maxRow) maxRow = row;
    }

    if (maxRow == -1) return 0;
    return (maxRow - minRow);
}

void CopyFormat1::AddEffect(Effect* eff, int row, EffectLayer* effectLayer)
{
    if (_pasteByCell && effectLayer != nullptr) {
        // if we don't add 1ms, it picks up the end of the previous timing instead of the start of this one
        Effect* te_start = effectLayer->GetEffectByTime(eff->GetStartTimeMS() + 1);
        Effect* te_end = effectLayer->GetEffectByTime(eff->GetEndTimeMS());

        if (te_start != nullptr && te_end != nullptr) {
            int start_pct = ((eff->GetStartTimeMS() - te_start->GetStartTimeMS()) * 100) / (te_start->GetEndTimeMS() - te_start->GetStartTimeMS());
            int end_pct = ((eff->GetEndTimeMS() - te_end->GetStartTimeMS()) * 100) / (te_end->GetEndTimeMS() - te_end->GetStartTimeMS());
            int start_index;
            int end_index;
            effectLayer->HitTestEffectByTime(te_start->GetStartTimeMS() + 1, start_index);
            effectLayer->HitTestEffectByTime(te_end->GetStartTimeMS() + 1, end_index);

            // override the start column with the start index if one wasnt set
            if (_startColumn == -1) _startColumn = start_index;

            _effects.push_back(new CopyFormat1Effect(eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), row, start_index, start_pct, end_index, end_pct, _startTime, eff->GetSettingsAsString(), eff->GetPaletteAsString()));
        }
        else {
            _effects.push_back(new CopyFormat1Effect(eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), row, eff->GetSettingsAsString(), eff->GetPaletteAsString()));
        }
    }
    else {
        _effects.push_back(new CopyFormat1Effect(eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), row, eff->GetSettingsAsString(), eff->GetPaletteAsString()));
    }
}

void CopyFormat1::AddTiming(Effect* eff, int row)
{
    _timings.push_back(new CopyFormat1Timing(eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), row));
}

size_t CopyFormat1::Frames(int frameMS)
{
    size_t startMS = StartTime();
    size_t endMS = EndTime();

    if (startMS >= endMS) return 0;
    return (endMS - startMS) / frameMS;
}

long CopyFormat1::StartTime() const
{
    size_t startMS = 9999999;

    for (const auto& it : _effects) {
        if (it->StartTime() < startMS) startMS = it->StartTime();
    }

    if (startMS == 9999999) return 0;
    return startMS;
}

long CopyFormat1::EndTime() const
{
    size_t endMS = 0;

    for (const auto& it : _effects) {
        if (it->EndTime() > endMS) endMS = it->EndTime();
    }

    return endMS;
}

COPYFORMAT CopyFormat::GetCopyFormat(const std::string& data)
{
    CopyFormat1 cf1(data);
    if (cf1.IsOk()) return COPYFORMAT::COPYFORMAT_1;

    //CopyFormatAC cfac(data);
    //if (cfac.IsOk()) return COPYFORMAT::COPYFORMAT_AC;

    //CopyFormatXL3 cfxl3(data);
    //if (cfxl3.IsOk()) return COPYFORMAT::COPYFORMAT_XL3;

    return COPYFORMAT::COPYFORMAT_INVALID;
}

CopyFormat1Effect::CopyFormat1Effect(const std::string& name, long start, long end, int row, const std::string& settings, const std::string& palette)
{
    _effectName = name;
    _pasteByCell = false;
    _startTime = start;
    _endTime = end;
    _row = row;
    _settings = settings;
    _palette = palette;
    _ok = true;
    _copyStartTime = -1000;
    _startCol = -1;
    _endCol = -1;
    _startPct = -1;
    _endPct = -1;
}

CopyFormat1Effect::CopyFormat1Effect(const std::string& name, long start, long end, int row, int startCol, int startPct, int endCol, int endPct, long copyStartTime, const std::string& settings, const std::string& palette)
{
    _effectName = name;
    _pasteByCell = true;
    _startTime = start;
    _endTime = end;
    _row = row;
    _settings = settings;
    _palette = palette;
    _ok = true;
    _copyStartTime = copyStartTime;
    _startCol = startCol;
    _endCol = endCol;
    _startPct = startPct;
    _endPct = endPct;
}

CopyFormat1Effect::CopyFormat1Effect(const std::string& data, bool pasteByCell)
{
    _ok = false;

    // parse the effect line
    wxArrayString cols = wxSplit(data, '\t');

    if (!pasteByCell) {
        if (cols.size() != 8) {
            // not the right number of columns
            return;
        }

        _effectName = cols[0].ToStdString();
        _settings = cols[1].ToStdString();
        _palette = cols[2].ToStdString();
        _startTime = wxAtoi(cols[3]);
        _endTime = wxAtoi(cols[4]);
        _row = wxAtoi(cols[5]);
        _copyStartTime = wxAtoi(cols[6]);
        _pasteByCell = false;
        if (cols[7] == "PASTE_BY_CELL") {
            _pasteByCell = true;
        }
    }
    else {
        if (cols.size() != 11) {
            // not the right number of columns
            return;
        }

        _effectName = cols[0].ToStdString();
        _settings = cols[1].ToStdString();
        _palette = cols[2].ToStdString();
        _startTime = wxAtoi(cols[3]);
        _endTime = wxAtoi(cols[4]);
        _row = wxAtoi(cols[5]);
        _copyStartTime = wxAtoi(cols[6]); // copy start time of earliest cell
        _startCol = wxAtoi(cols[7]); // which column the effect starts in
        _endCol = wxAtoi(cols[8]); // end index - which column the effect ends in
        _startPct = wxAtoi(cols[9]); // start pct - how far into the start column the effect starts
        _endPct = wxAtoi(cols[10]); // end pct - how far into the end column the effect ends

        _pasteByCell = false;
    }

    _ok = true;
}

std::string CopyFormat1Effect::Serialise() const
{
    if (!_pasteByCell) {
        wxString data = wxString::Format("%s\t%s\t%s\t%d\t%d\t%d\t%d\t%s", _effectName, _settings, _palette, _startTime, _endTime, _row, _copyStartTime, (_pasteByCell ? "PASTE_BY_CELL" : "NO_PASTE_BY_CELL"));
        return data.ToStdString();
    }
    else {
        wxString data = wxString::Format("%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
            _effectName,
            _settings,
            _palette,
            _startTime,
            _endTime,
            _row,
            _copyStartTime,
            _startCol,
            _endCol,
            _startPct,
            _endPct);
        return data.ToStdString();
    }
}

CopyFormat1Timing::CopyFormat1Timing(const std::string& data)
{
    _ok = false;

    // parse the effect line
    wxArrayString cols = wxSplit(data, '\t');

    if (cols.size() != 8) {
        // not the right number of columns
        return;
    }

    _timingName = cols[0].ToStdString();
    _startTime = wxAtoi(cols[3]);
    _endTime = wxAtoi(cols[4]);
    _row = wxAtoi(cols[5]);
    _columnStart = wxAtoi(cols[6]);
    wxASSERT(cols[7] == "TIMING_EFFECT");

    _ok = true;
}

CopyFormat1Timing::CopyFormat1Timing(const std::string& name, long start, long end, int row)
{
    _timingName = name;
    _startTime = start;
    _endTime = end;
    _row = row;
    _columnStart = -1000;
    _ok = true;
}

std::string CopyFormat1Timing::Serialise() const
{
    wxString data = wxString::Format("%s\t\t\t%d\t%d\t%d\t%d\tTIMING_EFFECT", _timingName, _startTime, _endTime, _row, _columnStart);
    return data.ToStdString();
}
