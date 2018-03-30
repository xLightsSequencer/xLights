#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <list>
#include <map>
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
    wxUint32 _id;
    std::list<PlayListStep*> _steps;
    std::list<Schedule*> _schedules;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _stopAtEndOfCurrentStep;
    std::string _commandAtEndOfCurrentStep;
    std::string _commandParametersAtEndOfCurrentStep;
    bool _firstOnlyOnce;
    bool _lastOnlyOnce;
    PlayListStep* _currentStep;
    wxDateTime _pauseTime;
    wxDateTime _suspendTime;
    bool _looping;
    bool _lastLoop;
    bool _loopStep;
    int _loops;
    bool _random;
    bool _jumpToEndStepsAtEndOfCurrentStep;
    std::string _forceNextStep;
    #pragma endregion Member Variables

    int GetPos(PlayListStep* step);
    PlayListStep* GetPriorStep() const;
    void ForgetChildren();
    void DeleteChildren();

public:

    #pragma region Constructors and Destructors
    PlayList(OutputManager* outputManager, wxXmlNode* node);
    PlayList(const PlayList& playlist, bool newid = false);
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
    void JumpToStepAtEndOfCurrentStep(const std::string& step) { _forceNextStep = step; }
    PlayListStep* GetNextStep(bool& didloop);
    PlayListStep* GetRunningStep() const { return _currentStep; }
    std::list<PlayListStep*> GetSteps() const { return _steps; }
    std::string GetNextScheduledTime() const;
    std::list<Schedule*> GetSchedules() const { return _schedules; }
    size_t GetLengthMS() const;
    bool IsDirty() const;
    void ClearDirty();
    PlayListItem* FindRunProcessNamed(const std::string& item) const;
    int GetFrameMS() const;
    Schedule* GetSchedule(int id) const;
    Schedule* GetSchedule(const std::string& name) const;
    int GetChangeCount() const { return _changeCount; }
    bool SupportsRandom() const;
    bool IsRandom() const { return _random; }
    bool SetRandom(bool random) { _random = random; return true; }
    bool SetLooping(bool looping) { _looping = looping; return true; }
    bool IsStepLooping() const { return _loopStep; }
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { --_loops; if (_loops == 0) { _loops = -1; _looping = false; } }
    void ClearStepLooping() { _loopStep = false; }
    PlayListStep* GetStepAtTime(long ms) const;
    size_t GetPosition() const;
    std::string GetName() const;
    std::string GetRawName() const {  return _name; };
    std::string GetNameNoTime() const { if (_name == "") return "<unnamed>"; else return _name; };
    void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    bool GetFirstOnce() const
    { return _firstOnlyOnce; }
    void SetFirstOnce(bool foo) { if (_firstOnlyOnce != foo) { _firstOnlyOnce = foo; _changeCount++; } }
    bool GetLastOnce() const
    { return _lastOnlyOnce; }
    void SetLastOnce(bool foo) { if (_lastOnlyOnce != foo) { _lastOnlyOnce = foo; _changeCount++; } }
    bool Frame(wxByte* buffer, size_t size, bool outputframe); // true if this was the last frame
    int GetPlayListSize() const { return _steps.size(); }
    bool IsLooping() const { return _looping; }
    void StopAtEndOfThisLoop() { _lastLoop = true; }
    bool IsSimple() const;
    std::string GetActiveSyncItemFSEQ() const;
    std::string GetActiveSyncItemMedia() const;
    void AddStep(PlayListStep* item, int pos);
    void RemoveStep(PlayListStep* item);
    void RemoveSchedule(Schedule* item);
    void MoveStepAfterStep(PlayListStep* movethis, PlayListStep* afterthis);
    void AddSchedule(Schedule* schedule);
    bool IsRunning() const;
    void Start(bool loop = false, bool random = false, int loops = -1, const std::string& step = "");
    void StartSuspended(bool loop = false, bool random = false, int loops = -1, const std::string& step = "");
    int Suspend(bool suspend);
    bool IsSuspended() const;
    void Stop();
    void StopAtEndOfCurrentStep() { _stopAtEndOfCurrentStep = true; }
    void TogglePause();
    void RemoveAllSteps();
    bool IsPaused() const;
    bool JumpToNextStep();
    bool MoveToNextStep();
    bool JumpToPriorStep();
    bool JumpToStep(const std::string& step);
    bool JumpToEndStepsAtEndOfCurrentStep();
    void RestartCurrentStep();
    PlayListStep* GetStepContainingPlayListItem(wxUint32 id) const;
    PlayListStep* GetStep(const std::string& step);
    PlayListStep* GetStep(wxUint32 id);
    PlayListItem* GetItem(wxUint32 id);
    PlayListStep* GetStepWithFSEQ(const std::string fseqFile);
    PlayListStep* GetStepWithTimingName(const std::string timingName);
    PlayListStep* GetRandomStep();
    bool LoopStep(const std::string step);
    PlayListItemText* GetRunningText(const std::string& name) const;
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(OutputManager* outputManager, wxXmlNode * node);
    
    #pragma region UI
    // returns nullptr if cancelled
    PlayList* Configure(wxWindow* parent, OutputManager* outputManager, bool advanced);
    #pragma endregion UI

};
#endif