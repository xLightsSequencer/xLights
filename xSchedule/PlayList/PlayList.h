#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <map>
#include <mutex>
#include <wx/wx.h>

class OutputManager;
class PlayListItemText;
class wxXmlNode;
class PlayListStep;
class wxWindow;
class Schedule;
class PlayListItem;

class PlayList
{
protected:

    #pragma region Member Variables
    int _reentrancyCounter;
    wxUint32 _id;
    std::list<PlayListStep*> _steps;
    std::list<PlayListStep*> _everySteps;
    std::list<Schedule*> _schedules;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _stopAtEndOfCurrentStep;
    std::string _commandAtEndOfCurrentStep;
    std::string _commandParametersAtEndOfCurrentStep;
    bool _firstOnlyOnce;
    bool _lastOnlyOnce;
    bool _alwaysShuffle;
    PlayListStep* _currentStep;
    wxDateTime _pauseTime;
    wxDateTime _suspendTime;
    bool _looping;
    bool _lastLoop;
    bool _loopStep;
    int _loops;
    bool _random;
    bool _suspendAtEndOfStep;
    bool _jumpToEndStepsAtEndOfCurrentStep;
    std::string _forceNextStep;
    std::list<wxUint32> _played;
    #pragma endregion Member Variables

    int GetPos(PlayListStep* step);
    PlayListStep* GetPriorStep();
    void ForgetChildren();
    void DeleteChildren();
    static bool IsInSlaveMode();
    bool JumpToStep(PlayListStep* pls);

public:

    #pragma region Constructors and Destructors
    PlayList(OutputManager* outputManager, wxXmlNode* node);
    PlayList(PlayList& playlist, bool newid = false);
    PlayList();
    virtual ~PlayList();
    #pragma endregion Constructors and Destructors

    bool operator==(const PlayList& rhs) const { return _id == rhs._id; }
    PlayList& operator=(PlayList& playlist);

    #pragma region Getters and Setters
    void RemoveEmptySteps();
    void SetCommandAtEndOfCurrentStep(const std::string& command, const std::string& parameters) { _commandAtEndOfCurrentStep = command; _commandParametersAtEndOfCurrentStep = parameters; }
    wxUint32 GetId() const { return _id; }
    bool IsFinishingUp() const { return _jumpToEndStepsAtEndOfCurrentStep; }
    void SetSuspendAtEndOfCurrentStep() { _suspendAtEndOfStep = true; }
    void JumpToStepAtEndOfCurrentStep(const std::string& step) { _forceNextStep = step; }
    PlayListStep* GetNextStep(bool& didloop);
    PlayListStep* GetRunningStep() const { return _currentStep; }
    std::list<PlayListStep*> GetSteps() const { return _steps; }
    int GetStepCount() const { return _steps.size(); }
    std::string GetNextScheduledTime();
    std::list<Schedule*> GetSchedules() const { return _schedules; }
    size_t GetLengthMS();
    bool IsDirty();
    void ClearDirty();
    PlayListItem* FindRunProcessNamed(const std::string& item);
    int GetFrameMS();
    Schedule* GetSchedule(int id);
    Schedule* GetSchedule(const std::string& name);
    int GetChangeCount() const { return _changeCount; }
    bool SupportsRandom();
    void SetPosition(long secs);
    bool IsRandom() const { return _random || _alwaysShuffle; }
    bool SetRandom(bool random) { _random = random; if (random) _played.clear(); return true; }
    bool SetLooping(bool looping) { _looping = looping; return true; }
    bool IsStepLooping() const { return _loopStep; }
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { --_loops; if (_loops == 1) { _loops = -1; _looping = false; } }
    void SetStepLooping(bool loop) { _loopStep = loop; }
    PlayListStep* GetStepAtTime(long ms, long& stepMS);
    size_t GetPosition();
    std::string GetName();
    std::string GetRawName() const {  return _name; };
    std::string GetNameNoTime() const { if (_name == "") return "<unnamed>"; else return _name; };
    void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    bool GetFirstOnce() const
    { return _firstOnlyOnce; }
    void SetFirstOnce(bool foo) { if (_firstOnlyOnce != foo) { _firstOnlyOnce = foo; _changeCount++; } }
    bool GetLastOnce() const { return _lastOnlyOnce; }
    bool GetShuffle() const { return _alwaysShuffle; }
    void SetLastOnce(bool foo) { if (_lastOnlyOnce != foo) { _lastOnlyOnce = foo; _changeCount++; } }
    void SetShuffle(bool foo) { if (_alwaysShuffle != foo) { _alwaysShuffle = foo; _changeCount++; } }
    bool Frame(uint8_t* buffer, size_t size, bool outputframe); // true if this was the last frame
    int GetPlayListSize() const { return _steps.size(); }
    bool IsLooping() const { return _looping; }
    void StopAtEndOfThisLoop() { _lastLoop = true; }
    void ClearStopAtEndOfThisLoop() { _lastLoop = false; }
    std::string GetStepStartTime(PlayListStep* step) const;
    bool IsSimple();
    std::string GetActiveSyncItemFSEQ();
    std::string GetActiveSyncItemMedia();
    void AddStep(PlayListStep* item, int pos);
    void RemoveStep(PlayListStep* item);
    void RemoveSchedule(Schedule* item);
    void MoveStepBeforeStep(PlayListStep* movethis, PlayListStep* beforethis);
    void AddSchedule(Schedule* schedule);
    bool IsRunning() const;
    void Start(bool loop = false, bool random = false, int loops = -1, const std::string& step = "");
    void StartSuspended(bool loop = false, bool random = false, int loops = -1, const std::string& step = "");
    int Suspend(bool suspend);
    bool IsSuspended() const;
    void Stop();
    void StopAtEndOfCurrentStep() { _stopAtEndOfCurrentStep = true; }
    void ClearStopAtEndOfCurrentStep() { _stopAtEndOfCurrentStep = false; }
    void TogglePause();
    void RemoveAllSteps();
    bool IsPaused() const;
    bool JumpToNextStep();
    bool MoveToNextStep(bool suppressNext);
    bool JumpToPriorStep();
    bool JumpToStep(const std::string& step);
    bool JumpToEndStepsAtEndOfCurrentStep();
    void RestartCurrentStep();
    PlayListStep* GetStepContainingPlayListItem(wxUint32 id);
    PlayListStep* GetStep(const std::string& step);
    PlayListStep* GetStep(wxUint32 id);
    PlayListItem* GetItem(wxUint32 id);
    PlayListStep* GetStepWithFSEQ(const std::string fseqFile);
    PlayListStep* GetStepWithTimingName(const std::string timingName);
    PlayListStep* GetRandomStep();
    bool LoopStep(const std::string step);
    PlayListItemText* GetRunningText(const std::string& name);
    #pragma endregion Getters and Setters

    void ConsolidateEveryDay();
    void SeparateEveryDay();

    wxXmlNode* Save();
    void Load(OutputManager* outputManager, wxXmlNode * node);
    
    #pragma region UI
    // returns nullptr if cancelled
    PlayList* Configure(wxWindow* parent, OutputManager* outputManager, bool advanced);
    #pragma endregion UI

};
