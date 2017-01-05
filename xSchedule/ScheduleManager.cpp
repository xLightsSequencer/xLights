#include "ScheduleManager.h"
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include "ScheduleOptions.h"
#include "PlayList/PlayList.h"
#include "../xLights/outputs/OutputManager.h"
#include "PlayList/PlayListStep.h"
#include "PlayList/PlayListItem.h"
#include <log4cpp/Category.hh>

ScheduleManager::ScheduleManager(const std::string& showDir)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Loading schedule from %s.", (const char *)_showDir.c_str());

    _immediatePlay = nullptr;
    _scheduleOptions = nullptr;
    _showDir = showDir;
    _startTime = wxGetUTCTimeMillis();
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
    else
    {
        logger_base.error("Problem loading xml file %s.", (const char *)(showDir + "/" + GetScheduleFile()).c_str());
    }

    if (_scheduleOptions == nullptr)
    {
        _scheduleOptions = new ScheduleOptions();
    }

    _outputManager = new OutputManager();
    _outputManager->Load(_showDir, _scheduleOptions->IsSync());
    logger_base.info("Loaded outputs from %s.", (const char *)(_showDir + "/" + _outputManager->GetNetworksFileName()).c_str());
    _outputManager->StartOutput();
    logger_base.info("Started outputting to lights.");

    // This is out frame data buffer ... it cannot be resized
    logger_base.info("Allocated frame buffer of %ld bytes", _outputManager->GetTotalChannels());
    _buffer = (wxByte*)malloc(_outputManager->GetTotalChannels());
    memset(_buffer, 0x00, _outputManager->GetTotalChannels());
}

ScheduleManager::~ScheduleManager()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _outputManager->StopOutput();
    logger_base.info("Stopped outputting to lights.");
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

    logger_base.info("Closed schedule.");
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
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
    logger_base.info("Saved Schedule to %s.", (const char*)(_showDir + "/" + GetScheduleFile()).c_str());
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Deleting playlist %s.", (const char*)playlist->GetName().c_str());
    _playLists.remove(playlist);
    _dirty = true;
}

PlayList* ScheduleManager::GetRunningPlayList() const
{
    // find the highest priority running playlist
    PlayList* running = nullptr;

    if (_immediatePlay != nullptr && _immediatePlay->IsRunning())
    {
        running = _immediatePlay;
    }
    else
    {
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
    }

    return running;
}

void ScheduleManager::StopAll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Stopping all playlists.");
    
    PlayList* running = GetRunningPlayList();
        while (running != nullptr)
        {
            running->Stop();
            running = GetRunningPlayList();
        }

        if (_immediatePlay != nullptr)
        {
            delete _immediatePlay;
            _immediatePlay = nullptr;
        }
}

void ScheduleManager::Frame()
{
    PlayList* running = GetRunningPlayList();

    if (running != nullptr)
    {
        long msec = (wxGetUTCTimeMillis() - _startTime).ToLong();
        _outputManager->StartFrame(msec);
        bool done = running->Frame(_buffer, _outputManager->GetTotalChannels());
        _outputManager->SetManyChannels(0, _buffer, _outputManager->GetTotalChannels());
        _outputManager->EndFrame();

        if (done)
        {
#pragma todo ... this is where I should decide to loop etc
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

int ScheduleManager::PlayPlayList(PlayList* playlist)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playing playlist %s.", (const char*)playlist->GetName().c_str());

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
    PlayList* curr = GetRunningPlayList();

    if (!IsSomethingPlaying())
    {
        return "Idle";
    }

    return "Playing " + curr->GetRunningStep()->GetName() + " Time: " + FormatTime(curr->GetRunningStep()->GetPosition()) + " Left: " + FormatTime(curr->GetRunningStep()->GetLengthMS() - curr->GetRunningStep()->GetPosition());
}