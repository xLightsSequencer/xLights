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

#include <string>
#include <list>

class EffectLayer;
class Effect;

typedef enum
{
    COPYFORMAT_1,
    COPYFORMAT_AC,
    COPYFORMAT_XL3,
    COPYFORMAT_INVALID
} COPYFORMAT;

class CopyFormat
{
public:
    static COPYFORMAT GetCopyFormat(const std::string& data);
};

class CopyFormat1Effect
{
    bool _ok;
    std::string _effectName;
    std::string _settings;
    std::string _palette;
    long _startTime;
    long _endTime;
    int _row;
    long _copyStartTime;
    bool _pasteByCell;
    int _startCol;
    int _endCol;
    int _startPct;
    int _endPct;

public:
    CopyFormat1Effect(const std::string& name, long start, long end, int row, const std::string& settings, const std::string &palette);
    CopyFormat1Effect(const std::string& name, long start, long end, int row, int startCol, int startPct, int endCol, int endPct, long copyStartTime, const std::string& settings, const std::string& palette);
    CopyFormat1Effect(const std::string& data, bool pasteByCell);
    virtual ~CopyFormat1Effect() {}
    bool IsOk() const { return _ok; }
    std::string Serialise() const;
    std::string EffectName() const { return _effectName; }
    std::string Settings() const { return _settings; }
    std::string Palette() const { return _palette; }
    int Row() const { return _row; }
    //long ColumnStart() const { return _copyStartTime; }
    //int StartCol() const { return _startCol; }
    //int EndCol() const { return _endCol; }
    //int StartColPct() const { return _startPct; }
    //int EndColPct() const { return _endPct; }
    //bool IsPasteByCell() const { return _pasteByCell; }
    long StartTime() const { return _startTime; }
    long EndTime() const { return _endTime; }
};

class CopyFormat1Timing
{
    bool _ok;
    std::string _timingName;
    long _startTime;
    long _endTime;
    int _row;
    int _columnStart;

public:
    CopyFormat1Timing(const std::string& data);
    CopyFormat1Timing(const std::string& name, long start, long end, int row);
    virtual ~CopyFormat1Timing() {}
    bool IsOk() const { return _ok; }
    std::string Serialise() const;
    //std::string TimingName() const { return _timingName; }
    long StartTime() const { return _startTime; }
    long EndTime() const { return _endTime; }
    int Row() const { return _row; }
    //int ColumnStart() const { return _columnStart; }
};

class CopyFormat1
{
    bool _ok;
    int _numTimingRows;
    int _lastRow;
    bool _pasteByCell;
    long _startTime;
    long _startColumn;
    std::list<CopyFormat1Effect*> _effects;
    std::list<CopyFormat1Timing*> _timings;

    int CalcTimingRows() const;

public:
    CopyFormat1();
    CopyFormat1(const std::string& data);
    virtual ~CopyFormat1();
    bool IsOk() const { return _ok; }
    std::string Serialise() const;
    //int NumberOfTimings() const { return (int)_timings.size(); }
    //int NumberOfEffects() const { return (int)_effects.size(); }
    int NumberOfTimingRows() const { return _numTimingRows; }
    //int LastRow() const { return _lastRow; }
    //long StartingColumn() const { return _startColumn; }
    //bool PasteByCell() const { return _pasteByCell; }
    void SetNumberOfTimingRows(int value) { _numTimingRows = value; }
    void SetStart(int column, long time) { _startColumn = column; _startTime = time; }
    void SetPasteByCell(bool pasteByCell = true) { _pasteByCell = pasteByCell; }
    void AddEffect(Effect* eff, int row, EffectLayer* effectLayer = nullptr);
    void AddTiming(Effect* eff, int row);
    size_t Frames(int frameMS = 50);
    std::list<CopyFormat1Effect*> Effects() const { return _effects; }
    long StartTime() const; // time of start of earliest starting effect
    long EndTime() const; // time of end of latest finishing effect
};
