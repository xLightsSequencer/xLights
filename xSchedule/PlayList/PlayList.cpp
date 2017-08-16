#include "PlayList.h"
#include "PlayListDialog.h"
#include "PlayListStep.h"
#include "../Schedule.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "PlayListSimpleDialog.h"
#include "../xScheduleMain.h"
#include "../xScheduleApp.h"

int __playlistid = 0;

bool compare_sched(const Schedule* first, const Schedule* second)
{
    return first->GetPriority() > second->GetPriority();
}

PlayList::PlayList(wxXmlNode* node)
{
    _commandAtEndOfCurrentStep = "";
    _commandParametersAtEndOfCurrentStep = "";
    _loops = -1;
    _id = __playlistid++;
    _forceNextStep = "";
    _jumpToEndStepsAtEndOfCurrentStep = false;
    _lastLoop = false;
    _looping = false;
    _random = false;
    _loopStep = false;
    _pauseTime.Set((time_t)0);
    _suspendTime.Set((time_t)0);
    _stopAtEndOfCurrentStep = false;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _currentStep = nullptr;
    Load(node);

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.info("Playlist created from XML %s 0x%lx.", (const char*)GetName().c_str(), this);
}

void PlayList::ForgetChildren()
{
    _steps.clear();
    _schedules.clear();
}

PlayList& PlayList::operator=(PlayList& playlist)
{
    _commandAtEndOfCurrentStep = "";
    _commandParametersAtEndOfCurrentStep = "";
    _lastSavedChangeCount = playlist._lastSavedChangeCount;
    _changeCount = playlist._changeCount;
    _firstOnlyOnce = playlist._firstOnlyOnce;
    _lastOnlyOnce = playlist._lastOnlyOnce;
    _name = playlist._name;
    _loops = playlist._loops;
    _id = playlist._id;

    RemoveAllSteps();

    for (auto it = playlist._steps.begin(); it != playlist._steps.end(); ++it)
    {
        _steps.push_back(*it);
    }

    //for (auto it = playlist._schedules.begin(); it != playlist._schedules.end(); ++it)
    //{
    //    _schedules.push_back(*it);
    //}

    // need to have the playlist we are copying from forget its children otherwise they belong to both of us and that will cause problems during deletion.
    playlist._steps.clear();

    return *this;
}

PlayList::PlayList(const PlayList& playlist, bool newid)
{
    // logging to try to catch a scheduler crash
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Copying playlist %s steps %d.", (const char*)playlist.GetName().c_str(), playlist.GetSteps().size());

    _commandAtEndOfCurrentStep = "";
    _commandParametersAtEndOfCurrentStep = "";
    _forceNextStep = "";
    _jumpToEndStepsAtEndOfCurrentStep = false;
    _loopStep = false;
    _lastLoop = false;
    _random = false;
    _looping = false;
    _lastSavedChangeCount = playlist._lastSavedChangeCount;
    _changeCount = playlist._changeCount;
    _currentStep = nullptr;
    _stopAtEndOfCurrentStep = false;
    _pauseTime.Set((time_t)0);
    _suspendTime.Set((time_t)0);
    _firstOnlyOnce = playlist._firstOnlyOnce;
    _lastOnlyOnce = playlist._lastOnlyOnce;
    _name = playlist._name;
    _loops = playlist._loops;
    if (newid)
    {
        _id = __playlistid++;
    }
    else
    {
        _id = playlist._id;
    }

    for (auto it = playlist._steps.begin(); it != playlist._steps.end(); ++it)
    {
        _steps.push_back(new PlayListStep(**it));
    }
}

PlayList::PlayList()
{
    _commandAtEndOfCurrentStep = "";
    _commandParametersAtEndOfCurrentStep = "";
    _loops = -1;
    _id = __playlistid++;
    _forceNextStep = "";
    _jumpToEndStepsAtEndOfCurrentStep = false;
    _loopStep = false;
    _lastLoop = false;
    _random = false;
    _looping = false;
    _pauseTime.Set((time_t)0);
    _suspendTime.Set((time_t)0);
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _currentStep = nullptr;
    _firstOnlyOnce = false;
    _lastOnlyOnce = false;
    _name = "";
    _stopAtEndOfCurrentStep = false;

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.info("New playlist created from nothing 0x%lx.", this);
}

void PlayList::DeleteChildren()
{
    RemoveAllSteps();

    while (_schedules.size() > 0)
    {
        auto toremove = _schedules.front();
        _schedules.remove(toremove);
        delete toremove;
    }
}

PlayList::~PlayList()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.info("Playlist being destroyed %s 0x%lx.", (const char*)GetName().c_str(), this);

    DeleteChildren();
}

void PlayList::RemoveAllSteps()
{
    while (_steps.size() > 0)
    {
        auto toremove = _steps.front();
        _steps.remove(toremove);
        delete toremove;
    }
}

wxXmlNode* PlayList::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PlayList");

    if (_firstOnlyOnce)
    {
        res->AddAttribute("FirstOnce", "TRUE");
    }

    if (_lastOnlyOnce)
    {
        res->AddAttribute("LastOnce", "TRUE");
    }

    res->AddAttribute("Name", _name);

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    for (auto it = _schedules.begin(); it != _schedules.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    return res;
}

void PlayList::Load(wxXmlNode* node)
{
    _steps.clear();
    _schedules.clear();
    _firstOnlyOnce = (node->GetAttribute("FirstOnce", "FALSE") == "TRUE");
    _lastOnlyOnce = (node->GetAttribute("LastOnce", "FALSE") == "TRUE");
    _name = node->GetAttribute("Name", "");

    for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "PlayListStep")
        {
            _steps.push_back(new PlayListStep(n));
        }
        else if (n->GetName() == "Schedule")
        {
            _schedules.push_back(new Schedule(n));
        }
    }

    _schedules.sort(compare_sched);
}

PlayList* PlayList::Configure(wxWindow* parent, bool advanced)
{
    if (advanced || !IsSimple())
    {
        PlayListDialog dlg(parent, this);

        if (dlg.ShowModal() == wxID_CANCEL)
        {
            return nullptr;
        }
    }
    else
    {
        PlayListSimpleDialog dlg(parent, this);

        if (dlg.ShowModal() == wxID_CANCEL)
        {
            return nullptr;
        }
    }

    return this;
}

bool PlayList::IsDirty() const
{
    bool res = _lastSavedChangeCount != _changeCount;

    auto it = _steps.begin();
    while (!res && it != _steps.end())
    {
        res = res || (*it)->IsDirty();
        ++it;
    }

    auto it2 = _schedules.begin();
    while (!res && it2 != _schedules.end())
    {
        res = res || (*it2)->IsDirty();
        ++it2;
    }

    return res;
}

void PlayList::AddSchedule(Schedule* schedule)
{
    _schedules.push_back(schedule);
    _schedules.sort(compare_sched);
    _changeCount++;
}

void PlayList::AddStep(PlayListStep* item, int pos)
{
    std::list<PlayListStep*> newsteps;

    bool inserted = false;
    int i = 0;
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (i == pos)
        {
            newsteps.push_back(item);
            inserted = true;
        }
        newsteps.push_back(*it);
        i++;
    }

    if (!inserted)
    {
        newsteps.push_back(item);
    }

    _steps = newsteps;

    // sort on priority
    //_steps.sort();
}

void PlayList::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    for (auto it = _schedules.begin(); it != _schedules.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}

void PlayList::RemoveStep(PlayListStep* step)
{
    _steps.remove(step);
    _changeCount++;
}

void PlayList::RemoveSchedule(Schedule* schedule)
{
    _schedules.remove(schedule);
    _changeCount++;
}

void PlayList::MoveStepAfterStep(PlayListStep* movethis, PlayListStep* afterthis)
{
    if (_steps.size() == 1 || (afterthis != nullptr && movethis->GetId() == afterthis->GetId())) return;

    if (afterthis == nullptr)
    {
        RemoveStep(movethis);
        _steps.push_front(movethis);
    }
    else
    {
        RemoveStep(movethis);
        int pos = GetPos(afterthis);
        if (pos == -1)
        {
            wxASSERT(false);
            _steps.push_back(movethis);
        }
        else
        {
            AddStep(movethis, pos + 1);
        }
    }

    _changeCount++;
}

int PlayList::GetPos(PlayListStep* step)
{
    int i = 0;
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetId() == step->GetId())
        {
            return i;
        }
        i++;
    }

    return -1;
}

bool PlayList::Frame(wxByte* buffer, size_t size, bool outputframe)
{
    if (_currentStep != nullptr)
    {
        if (IsPaused() || IsSuspended())
        {
            return false;
        }

        // This returns true if everything is done
        if (_currentStep->Frame(buffer, size, outputframe))
        {
            return !MoveToNextStep();
        }
    }

    return false;
}

bool PlayList::IsRunning() const
{
    return _currentStep != nullptr;
}

void PlayList::StartSuspended(bool loop, bool random, int loops, const std::string& step)
{
    Start(loop, random, loops, step);
    Suspend(true);
}

void PlayList::Start(bool loop, bool random, int loops, const std::string& step)
{
    if (IsRunning()) return;
    if (_steps.size() == 0) return;

    _loops = loops;
    _looping = loop;
    _random = random;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******** Playlist %s starting to play.", (const char*)GetName().c_str());

    _forceNextStep = "";
    _loopStep = false;
    _stopAtEndOfCurrentStep = false;
    _jumpToEndStepsAtEndOfCurrentStep = false;
    _lastLoop = false;
    _stopAtEndOfCurrentStep = false;

    if (step == "")
    {
        if (_random && !_firstOnlyOnce)
        {
            _currentStep = GetRandomStep();
        }
        else
        {
            _currentStep = _steps.front();
        }
    }
    else
    {
        _currentStep = GetStep(step);
        if (_currentStep == nullptr)
        {
            if (_random && !_firstOnlyOnce)
            {
                _currentStep = GetRandomStep();
            }
            else
            {
                _currentStep = _steps.front();
            }
        }
    }

    if (_currentStep == nullptr)
    {
        logger_base.warn("Playlist %s has no steps.", (const char*)GetName().c_str());
    }
    else
    {
        _currentStep->Start(-1);
    }
}

void PlayList::Stop()
{
    if (!IsRunning()) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("******** Playlist %s stopping.", (const char*)GetName().c_str());

    _currentStep->Stop();
    _currentStep = nullptr;
}

PlayListStep* PlayList::GetNextStep(bool& didloop)
{
    didloop = false;
    if (_stopAtEndOfCurrentStep) return nullptr;
    if (_currentStep == nullptr) return nullptr;

    // this will contain a step name if this is to be our forced next step
    if (_forceNextStep != "")
    {
        for (auto it = _steps.begin(); it != _steps.end(); ++it)
        {
            if (wxString((*it)->GetNameNoTime()).Lower() == wxString(_forceNextStep).Lower())
            {
                return *it;
            }
        }
    }

    // If we have a limit on step loops
    if (_currentStep->IsMoreLoops())
    {
        _currentStep->DoLoop();
        if (_currentStep->IsMoreLoops())
            return _currentStep;
    }

    // if we are looping on the current step just return it
    if (_loopStep)
    {
        return _currentStep;
    }

    // get the last step in the playlist
    PlayListStep* last = _steps.back();

    if (_random && !_lastLoop)
    {
        return GetRandomStep();
    }

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetId() == _currentStep->GetId())
        {
            ++it;
            if (_jumpToEndStepsAtEndOfCurrentStep)
            {
                if (_steps.size() == 1) return nullptr;

                if (_lastOnlyOnce)
                {
                    return last;
                }
                else
                {
                    return nullptr;
                }
            }
            // This handles looping
            else if (IsLooping() && (it == _steps.end() || (_lastOnlyOnce && (*it)->GetId() == last->GetId())) && !_lastLoop)
            {
                didloop = true;
                if (_firstOnlyOnce)
                {
                    if (_steps.size() == 1) return nullptr;
                    auto it2 = _steps.begin();
                    ++it2;
                    return *it2;
                }

                return _steps.front();
            }
            else if (it == _steps.end())
            {
                return nullptr;
            }
            else
            {
                return (*it);
            }
        }
    }
    return nullptr;
}

PlayListStep* PlayList::GetPriorStep() const
{
    if (_stopAtEndOfCurrentStep) return nullptr;
    if (_currentStep == nullptr) return nullptr;

    PlayListStep* last = _steps.back();
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetId() == _currentStep->GetId())
        {
            return last;
        }
        last = *it;
    }

    return nullptr;
}

size_t PlayList::GetLengthMS() const
{
    size_t length = 0;
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        length += (*it)->GetLengthMS();
    }

    return length;
}

bool PlayList::IsPaused() const
{
    wxDateTime zero((time_t)0);
    return (_pauseTime != zero);
}

bool PlayList::IsSuspended() const
{
    wxDateTime zero((time_t)0);
    return (_suspendTime != zero);
}

void PlayList::TogglePause()
{
    if (_currentStep == nullptr) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (IsPaused())
    {
        logger_base.info("        Unpausing playlist %s.", (const char*)GetName().c_str());
        _currentStep->AdjustTime(wxDateTime::Now() - _pauseTime);
        _pauseTime = wxDateTime((time_t)0);
    }
    else
    {
        logger_base.info("        Pausing playlist %s.", (const char*)GetName().c_str());
        _pauseTime = wxDateTime::Now();
    }

    _currentStep->Pause(IsPaused());
}

int PlayList::Suspend(bool suspend)
{
    if (_currentStep == nullptr) return 50;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!IsPaused())
    {
        if (!suspend && IsSuspended())
        {
            // unsuspend
            logger_base.info("         Playlist %s unsuspending.", (const char*)GetNameNoTime().c_str());
            _currentStep->AdjustTime(wxDateTime::Now() - _suspendTime);
            _suspendTime = wxDateTime((time_t)0);
        }
        else if (suspend && !IsSuspended())
        {
            // suspend
            logger_base.info("         Playlist %s suspending.", (const char*)GetNameNoTime().c_str());
            _suspendTime = wxDateTime::Now();

        }
    }

    _currentStep->Suspend(suspend);

    return _currentStep->GetFrameMS();
}

bool PlayList::JumpToPriorStep()
{
    bool success = false;

    if (_currentStep == nullptr) return false;

    _pauseTime = 0;
    _currentStep->Stop();
    _currentStep = GetPriorStep();

    if (_currentStep == nullptr) return false;

    _currentStep->Start(-1);

    return success;
}

bool PlayList::JumpToNextStep()
{
    bool success = true;

    _pauseTime = 0;
    _loopStep = false;
    _forceNextStep = "";

    if (_currentStep == nullptr) return false;

    _currentStep->Stop();
    bool didloop;
    _currentStep = GetNextStep(didloop);
    if (didloop) DoLoop();

    if (_currentStep == nullptr) return false;

    _currentStep->Start(-1);

    return success;
}

bool PlayList::MoveToNextStep()
{
    bool success = true;

    if (_currentStep == nullptr) return false;

    _currentStep->Stop();

    if (_commandAtEndOfCurrentStep != "")
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Step completed so running command: '%s' parameters: '%s'.", (const char *)_commandAtEndOfCurrentStep.c_str(), (const char *)_commandParametersAtEndOfCurrentStep.c_str());

        wxCommandEvent event(EVT_RUNACTION);
        event.SetString(_commandAtEndOfCurrentStep + "|" + _commandParametersAtEndOfCurrentStep);
        wxPostEvent(wxGetApp().GetTopWindow(), event);

        _commandAtEndOfCurrentStep = "";
        _commandParametersAtEndOfCurrentStep = "";
    }

    bool didloop;
    _currentStep = GetNextStep(didloop);
    if (didloop) DoLoop();

    _forceNextStep = "";

    if (_currentStep == nullptr) return false;

    _currentStep->Start(-1);

    return success;
}

void PlayList::RestartCurrentStep()
{
    _pauseTime = 0;
    _forceNextStep = "";
    _loopStep = false;
    if (_currentStep != nullptr) _currentStep->Restart();
}

bool PlayList::JumpToStep(const std::string& step)
{
    bool success = true;

    _lastLoop = false;
    _loopStep = false;
    _forceNextStep = "";

    if (_currentStep != nullptr && wxString(_currentStep->GetName()).Lower() == wxString(step).Lower())
    {
        _currentStep->Restart();
        return success;
    }

    _currentStep->Stop();

    _currentStep = GetStep(step);
    if (_currentStep == nullptr)
    {
        return false;
    }

    _currentStep->Start(-1);

    return success;
}

bool PlayList::JumpToEndStepsAtEndOfCurrentStep()
{
    _forceNextStep = "";
    _jumpToEndStepsAtEndOfCurrentStep = true;
    _lastLoop = true;
    _loopStep = false;

    return (_steps.size() > 1 && _lastOnlyOnce);
}

PlayListStep* PlayList::GetStep(const std::string& step)
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (wxString((*it)->GetNameNoTime()).Lower() == wxString(step).Lower()) return (*it);
    }

    return nullptr;
}

PlayListStep* PlayList::GetStep(wxUint32 step)
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetId() == step) return (*it);
    }

    return nullptr;
}

bool PlayList::SupportsRandom() const
{
    int count = _steps.size();

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
            if (_steps.front()->GetId() == (*it)->GetId() && _firstOnlyOnce)
            {
                count--;
            }
            else if (_steps.back()->GetId() == (*it)->GetId() && _lastOnlyOnce)
            {
                count--;
            }
            else
            {
                if ((*it)->GetExcludeFromRandom())
                {
                    count--;
                }
            }
    }

    // 3 is the minimum number of eligible steps to support random
    return (count > 3);
}

PlayListStep* PlayList::GetRandomStep()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (SupportsRandom())
    {
        int selected = rand() % _steps.size();
        auto it = _steps.begin();

        for (int i = 0; i < selected && it != _steps.end(); i++)
        {
            ++it;
        }

        // we dont want the same step so try again or a step that isnt meant to be random
        if (it == _steps.end() ||
            (_currentStep != nullptr && (*it)->GetId() == _currentStep->GetId()) || 
            (*it)->GetExcludeFromRandom() ||
            (_firstOnlyOnce && (*it)->GetId() == _steps.front()->GetId()) ||
            (_lastOnlyOnce && (*it)->GetId() == _steps.back()->GetId())
            )
        {
            return GetRandomStep();
        }

        logger_base.info("Playlist %s randomly chose step %d of %d.", (const char*)GetName().c_str(), selected, _steps.size());

        return *it;
    }
    else
    {
        logger_base.info("Playlist %s random wont work as there are not at least 4 eligible steps. Just taking next available.", (const char*)GetName().c_str());
        bool didloop;
        bool oldrandom = _random;
        _random = false;
        PlayListStep* next = GetNextStep(didloop);
        _random = oldrandom;
        return next;
    }
}

bool PlayList::LoopStep(const std::string step)
{
    if (_currentStep == nullptr || wxString(_currentStep->GetNameNoTime()).Lower() != wxString(step).Lower())
    {
        JumpToStep(step);
    }

    _forceNextStep = "";
    _loopStep = true;

    return true;
}

std::string PlayList::GetActiveSyncItemFSEQ() const
{
    if (_currentStep != nullptr)
    {
        return _currentStep->GetActiveSyncItemFSEQ();
    }
    return "";
}

std::string PlayList::GetActiveSyncItemMedia() const
{
    if (_currentStep != nullptr)
    {
        return _currentStep->GetActiveSyncItemMedia();
    }
    return "";
}

std::string PlayList::GetName() const
{
    std::string duration = "";
    if (GetLengthMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetLengthMS() / 1000.0).ToStdString() + "]";
    }

    return GetNameNoTime() + duration;
}

bool PlayList::IsSimple() const
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (!(*it)->IsSimple())
        {
            return false;
        }
    }

    return true;
}

Schedule* PlayList::GetSchedule(int id) const
{
    for (auto it = _schedules.begin(); it != _schedules.end(); ++it)
    {
        if ((*it)->GetId() == id) return *it;
    }

    return nullptr;
}

Schedule* PlayList::GetSchedule(const std::string& name) const
{
    for (auto it = _schedules.begin(); it != _schedules.end(); ++it)
    {
        if (wxString((*it)->GetName()).Lower() == wxString(name).Lower()) return *it;
    }

    return nullptr;
}

void PlayList::RemoveEmptySteps()
{
    std::list<PlayListStep*> toremove;

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetItems().size() == 0)
        {
            toremove.push_back(*it);
        }
    }

    for (auto it = toremove.begin(); it != toremove.end(); ++it)
    {
        _steps.remove(*it);
    }
}

PlayListItemText* PlayList::GetRunningText(const std::string& name) const
{
    if (_currentStep == nullptr) return nullptr;

    return _currentStep->GetTextItem(name);
}

int PlayList::GetFrameMS() const
{
    if (_currentStep == nullptr)
    {
        return 50;
    }
    else
    {
        return _currentStep->GetFrameMS();
    }
}

PlayListStep* PlayList::GetStepWithFSEQ(const std::string fseqFile)
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->IsRunningFSEQ(fseqFile))
        {
            return *it;
        }
    }

    return nullptr;
}

PlayListItem* PlayList::FindRunProcessNamed(const std::string& item) const
{
    PlayListItem *pli = nullptr;

    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        pli = (*it)->FindRunProcessNamed(item);

        if (pli != nullptr) break;
    }

    return pli;
}

PlayListStep* PlayList::GetStepContainingPlayListItem(wxUint32 id) const
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if ((*it)->GetItem(id) != nullptr)
        {
            return *it;
        }
    }

    return nullptr;
}

std::string PlayList::GetNextScheduledTime() const
{
    wxDateTime nextdt = wxDateTime((time_t)0);

    for (auto it = _schedules.begin(); it != _schedules.end(); ++it)
    {
        wxDateTime dt = (*it)->GetNextTriggerDateTime();

        if (dt != wxDateTime((time_t)0))
        {
            if (nextdt == wxDateTime((time_t)0))
            {
                nextdt = dt;
            }
            else
            {
                if (dt < nextdt)
                {
                    nextdt = dt;
                }
            }
        }
    }

    if (nextdt == wxDateTime((time_t)0))
    {
        return "";
    }
    else
    {
        return nextdt.Format("%Y-%m-%d %H:%M").ToStdString();
    }
}

PlayListItem* PlayList::GetItem(wxUint32 id)
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        auto i = (*it)->GetItem(id);
        if (i != nullptr) return i;
    }

    return nullptr;
}

