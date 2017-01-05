#include "PlayList.h"
#include "PlayListDialog.h"
#include "PlayListStep.h"
#include "../Schedule.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

PlayList::PlayList(wxXmlNode* node)
{
    _dirty = false;
    _currentStep = nullptr;
    Load(node);
}

PlayList::PlayList(const PlayList& playlist)
{
    _dirty = false;
    _currentStep = nullptr;
    _firstOnlyOnce = playlist._firstOnlyOnce;
    _lastOnlyOnce = playlist._lastOnlyOnce;
    _name = playlist._name;
    for (auto it = playlist._steps.begin(); it != playlist._steps.end(); ++it)
    {
        _steps.push_back(new PlayListStep(**it));
    }
    // dont copy the schedule as it wont be needed
}

PlayList::PlayList()
{
    _dirty = false;
    _currentStep = nullptr;
    _firstOnlyOnce = false;
    _lastOnlyOnce = false;
    _name = "";
}

PlayList::~PlayList()
{
    while (_steps.size() > 0)
    {
        auto toremove = _steps.front();
        _steps.remove(toremove);
        delete toremove;
    }

    while (_schedules.size() > 0)
    {
        auto toremove = _schedules.front();
        _schedules.remove(toremove);
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

    _dirty = false;

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
}

PlayList* PlayList::Configure(wxWindow* parent)
{
    PlayListDialog dlg(parent, this);

    if (dlg.ShowModal() == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}

bool PlayList::IsDirty() const
{
    bool res = _dirty;

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
    _dirty = true;
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
}

void PlayList::ClearDirty()
{
    _dirty = false;

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
    _dirty = true;
}

void PlayList::RemoveSchedule(Schedule* schedule)
{
    _schedules.remove(schedule);
    _dirty = true;
}

void PlayList::MoveStepAfterStep(PlayListStep* movethis, PlayListStep* afterthis)
{
    if (movethis == afterthis) return;

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
}

int PlayList::GetPos(PlayListStep* step)
{
    int i = 0;
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (*it == step)
        {
            return i;
        }
        i++;
    }

    return -1;
}

bool PlayList::Frame(wxByte* buffer, size_t size)
{
    if (_currentStep != nullptr)
    {
        // This returns true if everything is done
        if (_currentStep->Frame(buffer, size))
        {
            _currentStep->Stop();
            _currentStep = GetNextStep();
            if (_currentStep == nullptr)
            {
                return true;
            }
        }
    }

    return false;
}

bool PlayList::IsRunning() const
{
    return _currentStep != nullptr;
}

void PlayList::Start()
{
    if (IsRunning()) return;
    if (_steps.size() == 0) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playlist %s starting to play.", (const char*)GetName().c_str());

    _currentStep = _steps.front();
    _currentStep->Start();
}

void PlayList::Stop()
{
    if (!IsRunning()) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playlist %s stopping.", (const char*)GetName().c_str());

    _currentStep->Stop();
    _currentStep = nullptr;
}

PlayListStep* PlayList::GetNextStep() const
{
    for (auto it = _steps.begin(); it != _steps.end(); ++it)
    {
        if (*it == _currentStep)
        {
            ++it;

            if (it == _steps.end()) return nullptr;

            return (*it);
        }
    }

    return nullptr;
}

