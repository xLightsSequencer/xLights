#include "ScheduleManager.h"
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include "ScheduleOptions.h"
#include "PlayList/PlayList.h"
#include "../xLights/outputs/OutputManager.h"
#include "PlayList/PlayListStep.h"
#include "PlayList/PlayListItem.h"

ScheduleManager::ScheduleManager(const std::string& showDir)
{
    _immediatePlay = nullptr;
    _scheduleOptions = nullptr;
    _showDir = showDir;
    wxDateTime now;
    _startTime = now.Today().GetValue().ToLong();
    _outputManager = nullptr;
    _buffer = nullptr;

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    _dirty = false;
	wxXmlDocument doc;
	doc.Load(showDir + "/" + GetScheduleFile());

    if (doc.IsOk())
    {
        for (wxXmlNode* n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == "PlayList")
            {
                _playLists.push_back(new PlayList(n));
            }
            else if (n->GetName() == "Options")
            {
                _scheduleOptions = new ScheduleOptions(n);
            }
        }
    }

    if (_scheduleOptions == nullptr)
    {
        _scheduleOptions = new ScheduleOptions();
    }

    _outputManager = new OutputManager();
    _outputManager->Load(_showDir, _scheduleOptions->IsSync());
    _outputManager->StartOutput();

    // This is out frame data buffer ... it cannot be resized
    _buffer = (wxByte*)malloc(_outputManager->GetTotalChannels());
    memset(_buffer, 0x00, _outputManager->GetTotalChannels());
}

ScheduleManager::~ScheduleManager()
{
    _outputManager->StopOutput();
	if (IsDirty())
	{
		if (wxMessageBox("Unsaved changes to the schedule. Save now?", "Unsave changes", wxYES_NO) == wxID_YES)
		{
			Save();
		}
	}

    while (_playLists.size() > 0)
    {
        auto toremove = _playLists.front();
        _playLists.remove(toremove);
        delete toremove;
    }

    if (_immediatePlay != nullptr)
    {
        delete _immediatePlay;
        _immediatePlay = nullptr;
    }

    delete _scheduleOptions;
    delete _outputManager;
    free(_buffer);
}

bool ScheduleManager::IsDirty()
{
    bool res = _dirty;

    auto it = _playLists.begin();
    while (!res && it != _playLists.end())
    {
        res = res || (*it)->IsDirty();
        ++it;
    }

    res = res || _scheduleOptions->IsDirty();

    return res;
}

void ScheduleManager::Save()
{
	wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "xSchedule");
    doc.SetRoot(root);

    root->AddChild(_scheduleOptions->Save());

	for(auto it = _playLists.begin(); it != _playLists.end(); ++it)
	{
		root->AddChild((*it)->Save());
	}

    doc.Save(_showDir + "/" + GetScheduleFile());
    ClearDirty();
}

void ScheduleManager::ClearDirty()
{
    _dirty = false;

    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    _scheduleOptions->ClearDirty();
}

void ScheduleManager::RemovePlayList(PlayList* playlist)
{
    _playLists.remove(playlist);
    _dirty = true;
}

PlayList* ScheduleManager::GetRunningPlayList() const
{
    // find the highest priority running playlist
    PlayList* running = nullptr;

    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        if ((*it)->IsRunning())
        {
            if (running == nullptr)
            {
                running = *it;
            }
            else
            {
                if ((*it)->GetPriority() > running->GetPriority())
                {
                    running = *it;
                }
            }
        }
    }

    return running;
}

void ScheduleManager::StopAll()
{
    if (_immediatePlay != nullptr)
    {
        _immediatePlay->Stop();
        delete _immediatePlay;
        _immediatePlay = nullptr;
    }

    if (IsRunning())
    {
        PlayList* running = GetRunningPlayList();
        while (running != nullptr)
        {
            running->Stop();
            running = GetRunningPlayList();
        }
    }
}

void ScheduleManager::Frame()
{
    if (_immediatePlay != nullptr)
    {
        wxDateTime now;
        long msec = (now.Today().GetValue() - _startTime).ToLong();
        _outputManager->StartFrame(msec);
        bool done = _immediatePlay->Frame(_buffer, _outputManager->GetTotalChannels());
        _outputManager->SetManyChannels(0, _buffer, _outputManager->GetTotalChannels());
        _outputManager->EndFrame();

        if (done)
        {
            delete _immediatePlay;
            _immediatePlay = nullptr;
        }
    }
    else
    {
        if (IsRunning())
        {

            PlayList* running = GetRunningPlayList();

            if (running != nullptr)
            {
                wxDateTime now;
                long msec = (now.Today().GetValue() - _startTime).ToLong();
                _outputManager->StartFrame(msec);
                bool done = running->Frame(_buffer, _outputManager->GetTotalChannels());
                _outputManager->SetManyChannels(0, _buffer, _outputManager->GetTotalChannels());
                _outputManager->EndFrame();

                if (done)
                {
#pragma todo ... this is where I should decide to loop etc
                }
            }
        }
        else
        {
            if (_scheduleOptions->IsSendOffWhenNotRunning())
            {
                _outputManager->StartFrame(0);
                _outputManager->AllOff();
                _outputManager->EndFrame();
            }
        }
    }
}

int ScheduleManager::PlayPlayList(PlayList* playlist)
{
    if (_immediatePlay != nullptr)
    {
        _immediatePlay->Stop();
        delete _immediatePlay;
        _immediatePlay = nullptr;
    }

    // this needs to create a copy of everything ... including steps etc
    _immediatePlay = new PlayList(*playlist);
    _immediatePlay->Start();

    return 25; // always start fast
}

int ScheduleManager::CheckSchedule()
{
    #pragma todo this is meant to work out if to start something ... it returns the desired frame rate
    return 50;
}

std::string FormatTime(size_t timems)
{
    return wxString::Format(wxT("%i:%02i.%03i"), timems / 60000, (timems % 60000) / 1000, timems % 1000).ToStdString();
}

std::string ScheduleManager::GetStatus() const
{
    PlayList* curr = _immediatePlay;

    if (curr == nullptr)
    {
        curr = GetRunningPlayList();
    }

    if (curr == nullptr || curr->GetRunningStep() == nullptr)
    {
        return "Idle";
    }

    return "Playing " + curr->GetRunningStep()->GetName() + " Time: " + FormatTime(curr->GetRunningStep()->GetPosition()) + " Left: " + FormatTime(curr->GetRunningStep()->GetLengthMS() - curr->GetRunningStep()->GetPosition());
}