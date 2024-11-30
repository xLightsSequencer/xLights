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

#include <list>
#include <string>
#include <wx/wx.h>
#include <mutex>

class OutputManager;
class PlayListItemText;
class wxXmlNode;
class PlayListItem;
class wxWindow;
class AudioManager;
class PlayList;
class ScheduleOptions;

class PlayListStep
{
protected:

#pragma region Member Variables
    mutable int _reentrancyCounter;
    std::list<PlayListItem*> _items;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _excludeFromRandom;
    size_t _framecount;
    size_t _currentFrame = 0;
    wxUint32 _startTime;
    wxUint32 _pause;
    wxUint32 _id;
    wxUint32 _suspend;
    int _baseTimeCodeTime = -1;
    int _loops = 0;
    bool _everyStep = false;
    bool _everyStepExcludeFirst = false;
    bool _everyStepExcludeLast = false;
#pragma endregion Member Variables

    std::string FormatTime(size_t timems, bool ms = false) const;
    AudioManager* GetAudioManager(PlayListItem* pli) const;

public:

#pragma region Constructors and Destructors
    PlayListStep(OutputManager* outputManager, wxXmlNode* node);
    PlayListStep();
    virtual ~PlayListStep();
    PlayListStep(const PlayListStep& step);
    PlayListStep* Clone() const;
#pragma endregion Constructors and Destructors

    bool operator==(const PlayListStep& rhs) const { return _id == rhs._id; }
    static int GetStepIdFromName(const std::string& step);
    static std::string GetStepNameWithId(int id);
    
#pragma region Getters and Setters
    PlayListItemText* GetTextItem(const std::string& name);
    wxUint32 GetId() const { return _id; }
    PlayListItem* GetTimeSource(size_t& ms) const;
    std::list<PlayListItem*> GetItems();
    size_t GetCurrentFrame() const { return _currentFrame; }
    bool IsDirty();
    void ClearDirty();
    void SetDirty() { ++_changeCount; }
    std::string GetStatus(bool ms = false);
    bool GetExcludeFromRandom() const { return _excludeFromRandom; }
    void SetExcludeFromRandom(bool efr) { if (_excludeFromRandom != efr) { _excludeFromRandom = efr; ++_changeCount; } }
    std::string GetStartTime(PlayList* pl);
    std::string GetName(PlayList* pl);
    std::string GetNameNoTime();
    std::string GetRawName() const { return _name; }
    void SetName(const std::string& name) { if (_name != name) { _name = name; ++_changeCount; } }
    void SetBaseTimeCodeTime(int time) {
        if (_baseTimeCodeTime != time) {
            _baseTimeCodeTime = time;
            ++_changeCount;
        }
    }
    int GetBaseTimeCodeTime() const {
        return _baseTimeCodeTime;
    }
    bool IsBaseTimeCodeOverride() const {
        return _baseTimeCodeTime >= 0;
    }
    void Start(int _loops);
    bool IsSimple();
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { --_loops; }
    bool IsMoreLoops() const { return _loops > 0; }
    void SetLoops(int loops) { _loops = loops; }
    void SetEveryStep(bool everyStep) { if (_everyStep != everyStep) { _everyStep = everyStep; ++_changeCount; } }
    bool GetEveryStep(void) const { return _everyStep; }
    void SetEveryStepExcludeFirst(bool excludeFirst)
    {
        if (_everyStepExcludeFirst != excludeFirst) {
            _everyStepExcludeFirst = excludeFirst;
            ++_changeCount;
        }
    }
    bool GetEveryStepExcludeFirst(void) const
    {
        return _everyStepExcludeFirst;
    }
    void SetEveryStepExcludeLast(bool excludeLast)
    {
        if (_everyStepExcludeLast != excludeLast) {
            _everyStepExcludeLast = excludeLast;
            ++_changeCount;
        }
    }
    bool GetEveryStepExcludeLast(void) const
    {
        return _everyStepExcludeLast;
    }
    bool IsPaused() const
    {
        return _pause != 0;
    }
    void Stop();
    void Suspend(bool suspend);
    void Restart();
    void Pause(bool pause);
    virtual void Advance(int seconds);
    std::string GetFSEQTimeStamp() const;
    std::string GetActiveSyncItemFSEQ() const;
    std::string GetActiveSyncItemMedia() const;
    int GetPlayStepSize() const { return _items.size(); }
    void AddItem(PlayListItem* item) { _items.push_back(item); _items.sort(); ++_changeCount; }
    void RemoveItem(PlayListItem* item);
    bool Frame(uint8_t* buffer, size_t size, bool outputframe);
    size_t GetPosition();
    PlayListItem* GetItem(const std::string item);
    PlayListItem* GetItem(const wxUint32 id);
    size_t GetLengthMS();
    size_t GetFrameMS();
    void AdjustTime(wxTimeSpan by);
    bool IsRunningFSEQ(const std::string& fseqFile);
    void SetSyncPosition(size_t ms, size_t acceptableJitter, bool force = false);
    PlayListItem* FindRunProcessNamed(const std::string& item);
    AudioManager* GetAudioManager();
    bool SetPosition(size_t frame, size_t ms);
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(OutputManager* outputManager, wxXmlNode * node);
};
