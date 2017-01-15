#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <list>
#include <map>
#include <wx/wx.h>

class wxXmlNode;
class PlayListStep;
class wxWindow;
class Schedule;

class PlayList
{
protected:

    #pragma region Member Variables
    wxLongLong _id;
    std::list<PlayListStep*> _steps;
    std::list<Schedule*> _schedules;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _stopAtEndOfCurrentStep;
    bool _firstOnlyOnce;
    bool _lastOnlyOnce;
    PlayListStep* _currentStep;
    int _priority;
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
    PlayList(wxXmlNode* node);
    PlayList(const PlayList& playlist);
    PlayList();
    virtual ~PlayList();
    #pragma endregion Constructors and Destructors

    bool operator==(const PlayList& rhs) const { return _id == rhs._id; }
    PlayList& operator=(PlayList& playlist);

    #pragma region Getters and Setters
    bool IsFinishingUp() const { return _jumpToEndStepsAtEndOfCurrentStep; }
    void JumpToStepAtEndOfCurrentStep(const std::string& step) { _forceNextStep = step; }
    PlayListStep* GetNextStep(bool& didloop);
    PlayListStep* GetRunningStep() const { return _currentStep; }
    std::list<PlayListStep*> GetSteps() const { return _steps; }
    std::list<Schedule*> GetSchedules() const { return _schedules; }
    size_t GetLengthMS() const;
    bool IsDirty() const;
    void ClearDirty();
    bool IsRandom() const { return _random; }
    bool SetRandom(bool random) { _random = random; return true; }
    bool SetLooping(bool looping) { _looping = looping; return true; }
    bool IsStepLooping() const { return _loopStep; }
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { --_loops; if (_loops == 0) { _loops = -1; _looping = false; } }
    void ClearStepLooping() { _loopStep = false; }
    std::string GetName() const { return _name; }
    std::string GetNameNoTime() const { return GetName(); };
    void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    bool GetFirstOnce() const
    { return _firstOnlyOnce; }
    void SetFirstOnce(bool foo) { if (_firstOnlyOnce != foo) { _firstOnlyOnce = foo; _changeCount++; } }
    bool GetLastOnce() const
    { return _lastOnlyOnce; }
    void SetLastOnce(bool foo) { if (_lastOnlyOnce != foo) { _lastOnlyOnce = foo; _changeCount++; } }
    int GetPriority() const { return _priority; }
    void SetPriority(int priority) { if (_priority != priority) { _priority = priority; _changeCount++; } }
    bool Frame(wxByte* buffer, size_t size); // true if this was the last frame
    int GetPlayListSize() const { return _steps.size(); }
    bool IsLooping() const { return _looping; }
    void StopAtEndOfThisLoop() { _lastLoop = true; }
    void AddStep(PlayListStep* item, int pos);
    void RemoveStep(PlayListStep* item);
    void RemoveSchedule(Schedule* item);
    void MoveStepAfterStep(PlayListStep* movethis, PlayListStep* afterthis);
    void AddSchedule(Schedule* schedule);
    bool IsRunning() const;
    void Start(bool loop = false, bool random = false, int loops = -1);
    void StartSuspended(bool loop = false, bool random = false, int loops = -1);
    int Suspend(bool suspend);
    bool IsSuspended() const;
    void Stop();
    void StopAtEndOfCurrentStep() { _stopAtEndOfCurrentStep = true; }
    void Pause();
    bool IsPaused() const;
    bool JumpToNextStep();
    bool MoveToNextStep();
    bool JumpToPriorStep();
    bool JumpToStep(const std::string& step);
    bool JumpToEndStepsAtEndOfCurrentStep();
    void RestartCurrentStep();
    PlayListStep* GetStep(const std::string& step);
    PlayListStep* GetRandomStep();
    bool LoopStep(const std::string step);
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(wxXmlNode * node);
    
    #pragma region UI
    // returns nullptr if cancelled
    PlayList* Configure(wxWindow* parent);
    #pragma endregion UI

};
#endif