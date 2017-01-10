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
    std::list<PlayListStep*> _steps;
    std::list<Schedule*> _schedules;
    bool _dirty;
    std::string _name;
    bool _stopAtEndOfCurrentStep;
    bool _firstOnlyOnce;
    bool _lastOnlyOnce;
    PlayListStep* _currentStep;
    int _priority;
    wxDateTime _pauseTime;
    bool _looping;
    bool _lastLoop;
    bool _loopStep;
    bool _random;
    bool _jumpToEndStepsAtEndOfCurrentStep;
    std::string _forceNextStep;
    #pragma endregion Member Variables

    int GetPos(PlayListStep* step);
    PlayListStep* GetPriorStep() const;

public:

    #pragma region Constructors and Destructors
    PlayList(wxXmlNode* node);
    PlayList(const PlayList& playlist);
    PlayList();
    virtual ~PlayList();
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    void JumpToStepAtEndOfCurrentStep(const std::string& step) { _forceNextStep = step; }
    PlayListStep* GetNextStep() const;
    PlayListStep* GetRunningStep() const { return _currentStep; }
    std::list<PlayListStep*> GetSteps() const { return _steps; }
    std::list<Schedule*> GetSchedules() const { return _schedules; }
    size_t GetLengthMS() const;
    bool IsDirty() const;
    void ClearDirty();
    bool IsRandom() const { return _random; }
    void SetRandom(bool random) { _random = random; _looping = false; }
    void SetLooping(bool looping) { _looping = looping; _random = false; }
    bool IsStepLooping() const { return _loopStep; }
    void ClearStepLooping() { _loopStep = false; }
    std::string GetName() const { return _name; }
    std::string GetNameNoTime() const;
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    bool GetFirstOnce() const
    { return _firstOnlyOnce; }
    void SetFirstOnce(bool foo) { _firstOnlyOnce = foo; _dirty = true; }
    bool GetLastOnce() const
    { return _lastOnlyOnce; }
    void SetLastOnce(bool foo) { _lastOnlyOnce = foo; _dirty = true; }
    int GetPriority() const { return _priority; }
    void SetPriority(int priority) { _priority = priority; _dirty = true; }
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
    void Start(bool loop = false, bool random = false);
    void Stop();
    void StopAtEndOfCurrentStep() { _stopAtEndOfCurrentStep = true; }
    void Pause();
    bool IsPaused() const;
    bool JumpToNextStep();
    bool JumpToPriorStep();
    bool JumpToStep(const std::string& step);
    bool JumpToEndStepsAtEndOfCurrentStep();
    void RestartCurrentStep();
    PlayListStep* GetStep(const std::string& step);
    PlayListStep* GetRandomStep() const;
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