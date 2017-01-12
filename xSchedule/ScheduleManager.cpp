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
#include <wx/dir.h>
#include <wx/file.h>

bool compare_name(const PlayList* first, const PlayList* second)
{
    return first->GetNameNoTime() < second->GetNameNoTime();
}

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

    _playLists.sort(compare_name);

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

void ScheduleManager::AddPlayList(PlayList* playlist)
{
    _playLists.push_back(playlist);     
    _playLists.sort(compare_name); 
    _dirty = true;
}

std::list<PlayList*> ScheduleManager::GetPlayLists()
{
    _playLists.sort(compare_name); 
    return _playLists;
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
    logger_base.info("Deleting playlist %s.", (const char*)playlist->GetNameNoTime().c_str());
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
            // playlist is done
            StopPlayList(running, false);
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

bool ScheduleManager::PlayPlayList(PlayList* playlist, size_t& rate, bool loop, const std::string& step, bool forcelast)
{
    bool result = true;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playing playlist %s.", (const char*)playlist->GetNameNoTime().c_str());

    if (_immediatePlay != nullptr)
    {
        _immediatePlay->Stop();
        delete _immediatePlay;
        _immediatePlay = nullptr;
    }

    // this needs to create a copy of everything ... including steps etc
    _immediatePlay = new PlayList(*playlist);
    _immediatePlay->Start(loop);
    if (step != "")
    {
        _immediatePlay->JumpToStep(step);
    }

    if (forcelast)
    {
        _immediatePlay->StopAtEndOfCurrentStep();
    }

    rate = 25; // always start fast
    return result;
}

int ScheduleManager::CheckSchedule()
{
    #pragma todo this is meant to work out if to start something ... it returns the desired frame rate
    return 50;
}

std::string ScheduleManager::FormatTime(size_t timems)
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

    return "Playing " + curr->GetRunningStep()->GetNameNoTime() + " " + curr->GetRunningStep()->GetStatus();
}

std::list<std::string> ScheduleManager::GetCommands() const
{
    std::list<std::string> res;

    res.push_back("Stop all now");
    res.push_back("Play selected playlist");
    res.push_back("Play selected playlist looped");
    res.push_back("Play specified playlist");
    res.push_back("Play specified playlist looped");
    res.push_back("Stop specified playlist");
    res.push_back("Stop specified playlist at end of current step");
    res.push_back("Stop specified playlist at end of current loop");
    res.push_back("Stop playlist at end of current step");
    res.push_back("Stop playlist at end of current loop");
    res.push_back("Jump to play once at end steps at end of current step and then stop");
    res.push_back("Pause");
    res.push_back("Next step in current playlist");
    res.push_back("Restart step in current playlist");
    res.push_back("Prior step in current playlist");
    res.push_back("Jump to random step in current playlist");
    res.push_back("Jump to random step in specified playlist");
    res.push_back("Jump to specified step in current playlist");
    res.push_back("Jump to specified step in current playlist at the end of current step");
    res.push_back("Play playlist starting at step");
    res.push_back("Play playlist starting at step looped");
    res.push_back("Toggle loop current step");
    res.push_back("Play specified step in specified playlist looped");
    res.push_back("Add to the specified schedule n minutes");
    res.push_back("Set volume to");
    res.push_back("Adjust volume by");
    res.push_back("Save schedule");
    res.push_back("Toggle output to lights");
    res.push_back("Toggle current playlist random");
    res.push_back("Toggle current playlist loop");
    res.push_back("Play specified playlist step once only");

    return res;
}

PlayList* ScheduleManager::GetPlayList(const std::string& playlist) const
{
    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        if ((*it)->GetNameNoTime() == playlist)
        {
            return *it;
        }
    }

    return nullptr;
}

// 127.0.0.1/xScheduleCommand?Command=Stop all now&Parameters=
// 127.0.0.1/xScheduleCommand?Command=Play specified playlist&Parameters=<play list name>
// 127.0.0.1/xScheduleCommand?Command=Stop specified playlist&Parameters=<play list name>
// 127.0.0.1/xScheduleCommand?Command=Stop specified playlist at end of current step&Parameters=<play list name>
// 127.0.0.1/xScheduleCommand?Command=Pause&Parameters=
// 127.0.0.1/xScheduleCommand?Command=Next step in current playlist&Parameters=
// 127.0.0.1/xScheduleCommand?Command=Restart step in current playlist&Parameters=
// 127.0.0.1/xScheduleCommand?Command=Prior step in current playlist&Parameters=
// 127.0.0.1/xScheduleCommand?Command=Play playlist starting at step&Parameters=<step name>
// 127.0.0.1/xScheduleCommand?Command=PressButton&Parameters=<button label>

bool ScheduleManager::Action(const std::string command, const std::string parameters, PlayList* playlist, size_t& rate, std::string& msg)
{
    bool result = true;

    if (command == "Stop all now")
    {
        StopAll();
    }
    else if (command == "Play selected playlist")
    {
        if (playlist != nullptr)
        {
            if(!PlayPlayList(playlist, rate))
            {
                result = false;
                msg = "Unable to start playlist.";
            }
        }
        else
        {
            result = false;
            msg = "No playlist selected.";
        }
    }
    else if (command == "Play selected playlist looped")
    {
        if (playlist != nullptr)
        {
            if (!PlayPlayList(playlist, rate, true))
            {
                result = false;
                msg = "Unable to start playlist.";
            }
        }
        else
        {
            result = false;
            msg = "No playlist selected.";
        }
    }
    else if (command == "Play specified playlist")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            if (!PlayPlayList(p, rate))
            {
                result = false;
                msg = "Unable to start playlist.";
            }
        }
        else
        {
            result = false;
            msg = "Playlist '"+parameters+"' not found.";
        }
    }
    else if (command == "Play specified playlist looped")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            if (!PlayPlayList(p, rate, true))
            {
                result = false;
                msg = "Unable to start playlist.";
            }
        }
        else
        {
            result = false;
            msg = "Playlist '" + parameters + "' not found.";
        }
    }
    else if (command == "Stop specified playlist")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            StopPlayList(p, false);
        }
        else
        {
            result = false;
            msg = "Playlist '" + parameters + "' not found.";
        }
    }
    else if (command == "Stop specified playlist at end of current step")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            StopPlayList(p, true);
        }
    }
    else if (command == "Stop playlist at end of current step")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            StopPlayList(p, true);
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Stop specified playlist at end of current loop")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            p->StopAtEndOfThisLoop();
        }
        else
        {
            result = false;
            msg = "Playlist '" + parameters + "' not found.";
        }
    }
    else if (command == "Jump to play once at end steps at end of current step and then stop")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            p->JumpToEndStepsAtEndOfCurrentStep();
        }
        else
        {
            result = false;
            msg = "No playlist currently running.";
        }
    }
    else if (command == "Stop playlist at end of current loop")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            p->StopAtEndOfThisLoop();
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Pause")
    {
        result = ToggleCurrentPlayListPause(msg);
    }
    else if (command == "Next step in current playlist")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            rate = p->JumpToNextStep();
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Restart step in current playlist")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            p->RestartCurrentStep();
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Prior step in current playlist")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            rate = p->JumpToPriorStep();
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Toggle loop current step")
    {
        result = ToggleCurrentPlayListStepLoop(msg);
    }
    else if (command == "Play specified step in specified playlist looped")
    {
        wxString parameter = parameters;
        wxArrayString split = wxSplit(parameter, ',');
        if (split.Count() != 2)
        {
            result = false;
            msg = "Parameters format incorrect.";
        }
        else
        {
            std::string pl = split[0].ToStdString();
            std::string step = split[1].ToStdString();

            PlayList* p = GetPlayList(pl);

            if (p != nullptr)
            {
                if (!PlayPlayList(p, rate, false, step))
                {
                    result = false;
                    msg = "Unable to start playlist.";
                }
                else
                {
                    p->LoopStep(step);
                }
            }
            else
            {
                result = false;
                msg = "Playlist '" + pl + "' not found.";
            }
        }
    }
    else if (command == "Play specified playlist step once only")
    {
        wxString parameter = parameters;
        wxArrayString split = wxSplit(parameter, ',');
        if (split.Count() != 2)
        {
            result = false;
            msg = "Parameters format incorrect.";
        }
        else
        {
            std::string pl = split[0].ToStdString();
            std::string step = split[1].ToStdString();

            PlayList* p = GetPlayList(pl);

            if (p != nullptr)
            {
                if (!PlayPlayList(p, rate, false, step, true))
                {
                    result = false;
                    msg = "Unable to start playlist.";
                }
            }
            else
            {
                result = false;
                msg = "Playlist '" + pl + "' not found.";
            }
        }
    }
    else if (command == "Play playlist starting at step")
    {
        wxString parameter = parameters;
        wxArrayString split = wxSplit(parameter, ',');
        if (split.Count() != 2)
        {
            result = false;
            msg = "Parameters format incorrect.";
        }
        else
        {
            std::string pl = split[0].ToStdString();
            std::string step = split[1].ToStdString();

            PlayList* p = GetPlayList(pl);

            if (p != nullptr)
            {
                if (!PlayPlayList(p, rate, false, step))
                {
                    result = false;
                    msg = "Unable to start playlist.";
                }
            }
            else
            {
                result = false;
                msg = "Playlist '" + pl + "' not found.";
            }
        }
    }
    else if (command == "Play playlist starting at step looped")
    {
        wxString parameter = parameters;
        wxArrayString split = wxSplit(parameter, ',');
        if (split.Count() != 2)
        {
            result = false;
            msg = "Parameters format incorrect.";
        }
        else
        {
            std::string pl = split[0].ToStdString();
            std::string step = split[1].ToStdString();

            PlayList* p = GetPlayList(pl);

            if (p != nullptr)
            {
                if (!PlayPlayList(p, rate, true, step))
                {
                    result = false;
                    msg = "Unable to start playlist.";
                }
            }
            else
            {
                result = false;
                msg = "Playlist '" + pl + "' not found.";
            }
        }
    }
    else if (command == "Jump to specified step in current playlist")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            rate = p->JumpToStep(parameters);
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Jump to specified step in current playlist at the end of current step")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            p->JumpToStepAtEndOfCurrentStep(parameters);
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Jump to random step in current playlist")
    {
        PlayList* p = GetRunningPlayList();

        if (p != nullptr)
        {
            auto r = p->GetRandomStep();
            if (r != nullptr)
            {
                rate = p->JumpToStep(r->GetNameNoTime());
            }
        }
        else
        {
            result = false;
            msg = "No playlist currently playing.";
        }
    }
    else if (command == "Jump to random step in specified playlist")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            auto r = p->GetRandomStep();
            if (r != nullptr)
            {
                rate = p->JumpToStep(r->GetNameNoTime());
            }
        }
        else
        {
            result = false;
            msg = "Playlist '" + parameters + "' not found.";
        }
    }
    else if (command == "Add to the specified schedule n minutes")
    {
#pragma todo need to add this    
    }
    else if (command == "Set volume to")
    {
#pragma todo need to add this    
    }
    else if (command == "Adjust volume by")
    {
#pragma todo need to add this    
    }
    else if (command == "Toggle output to lights")
    {
        result = ToggleOutputToLights(msg);
    }
    else if (command == "Toggle current playlist random")
    {
        result = ToggleCurrentPlayListRandom(msg);
    }
    else if (command == "Toggle current playlist loop")
    {
        result = ToggleCurrentPlayListLoop(msg);
    }
    else if (command == "Save schedule")
    {
        Save();
    }
    else if (command == "PressButton")
    {
        std::string c = _scheduleOptions->GetButtonCommand(parameters);
        std::string p = _scheduleOptions->GetButtonParameter(parameters);

        if (c != "")
        {
            result = Action(c, p, playlist, rate, msg);
        }
    }
    else
    {
        result = false;
        msg = "Unknown command.";
    }

    if (!result)
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Action failed: %s", (const char *)msg.c_str());
    }

    return result;
}

bool ScheduleManager::Action(const std::string label, PlayList* playlist, size_t& rate, std::string& msg)
{
    bool result = true;

    std::string command = _scheduleOptions->GetButtonCommand(label);
    std::string parameters = _scheduleOptions->GetButtonParameter(label);

    result = Action(command, parameters, playlist, rate, msg);

    return result;
}

void ScheduleManager::StopPlayList(PlayList* playlist, bool atendofcurrentstep)
{
    std::string name = playlist->GetNameNoTime();

    if (_immediatePlay != nullptr && _immediatePlay->GetNameNoTime() == name)
    {
        if (atendofcurrentstep)
        {
            _immediatePlay->StopAtEndOfCurrentStep();
        }
        else
        {
            _immediatePlay->Stop();
            delete _immediatePlay;
            _immediatePlay = nullptr;
        }
    }

    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        if ((*it)->GetNameNoTime() == name && (*it)->IsRunning())
        {
            if (atendofcurrentstep)
            {
                (*it)->StopAtEndOfCurrentStep();
            }
            else
            {
                (*it)->Stop();
            }
        }
    }
}

// 127.0.0.1/xScheduleStash?Command=Store&Key=<key> ... this must be posted with the data in the body of the request ... key must be filename legal
// 127.0.0.1/xScheduleStash?Command=Retrieve&Key=<key> ... this returs a text response with the data if successful

// 127.0.0.1/xScheduleQuery?Query=GetPlayLists&Parameters=
// 127.0.0.1/xScheduleQuery?Query=GetPlayListSteps&Parameters=<playlistname>
// 127.0.0.1/xScheduleQuery?Query=GetPlayingStatus&Parameters=
// 127.0.0.1/xScheduleQuery?Query=GetButtons&Parameters=

bool ScheduleManager::Query(const std::string command, const std::string parameters, std::string& data, std::string& msg)
{
    bool result = true;
    data = "";
    if (command == "GetPlayLists")
    {
        data = "{\"playlists\":[";
        for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
        {
            if (it != _playLists.begin())
            {
                data += ",";
            }
            data += "{\"name\":\"" + (*it)->GetNameNoTime() + "\",\"length\":\""+ FormatTime((*it)->GetLengthMS()) +"\"}";
        }
        data += "]}";
    }
    else if (command == "GetPlayListSteps")
    {
        PlayList* p = GetPlayList(parameters);

        if (p != nullptr)
        {
            data = "{\"steps\":[";
            auto steps = p->GetSteps();
            for (auto it =  steps.begin(); it != steps.end(); ++it)
            {
                if (it != steps.begin())
                {
                    data += ",";
                }
                data += "{\"name\":\"" + (*it)->GetNameNoTime() + "\",\"length\":\""+FormatTime((*it)->GetLengthMS())+"\"}";
            }
            data += "]}";
        }
        else
        {
            data = "{\"steps\":[]}";
            result = false;
            msg = "Playlist '" + parameters + "' not found.";
        }
    }
    else if (command == "GetPlayingStatus")
    {
        PlayList* p = GetRunningPlayList();
        if (p == nullptr)
        {
            data = "{\"status\":\"idle\",\"outputtolights\":\"" + std::string(_outputManager->IsOutputting() ? "true" : "false") + "\"}";
        }
        else
        {
            std::string nextsong = "";
            auto next = p->GetNextStep();
            if (p->IsRandom())
            {
                nextsong = "God knows";
            }
            else
            {
                nextsong = next->GetNameNoTime();
            }

            data = "{\"status\":\"" + std::string(p->IsPaused() ? "paused" : "playing") + "\",\"playlist\":\"" + p->GetNameNoTime() + 
                "\",\"playlistlooping\":\"" + (p->IsLooping() ? "true" : "false") +
                "\",\"random\":\"" + (p->IsRandom() ? "true" : "false") +
                "\",\"step\":\"" + p->GetRunningStep()->GetNameNoTime() +
                "\",\"steplooping\":\"" + (p->IsStepLooping()? "true" : "false") +
                "\",\"length\":\"" + FormatTime(p->GetRunningStep()->GetLengthMS()) +
                "\",\"position\":\"" + FormatTime(p->GetRunningStep()->GetPosition()) +
                "\",\"left\":\"" + FormatTime(p->GetRunningStep()->GetLengthMS() - p->GetRunningStep()->GetPosition()) + 
                "\",\"trigger\":\"" + std::string(IsCurrentPlayListScheduled() ? "scheduled": "manual") +
                "\",\"nextstep\":\"" + nextsong +
                "\",\"outputtolights\":\"" + std::string(_outputManager->IsOutputting() ? "true" : "false") + "\"}";
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("%s", (const char*)data.c_str());
        }
    }
    else if (command == "GetButtons")
    {
        data = _scheduleOptions->GetButtonsJSON();
    }
    else
    {
        result = false;
        msg = "Unknown query.";
    }

    return result;
}

bool ScheduleManager::StoreData(const std::string& key, const std::string& data, std::string& msg) const
{
    bool result = true;

    if (!wxDir::Exists(_showDir + "/xScheduleData"))
    {
        wxDir sd(_showDir);
        sd.Make(_showDir + "/xScheduleData");
    }

    std::string filename = _showDir + "/xScheduleData/" + key + ".dat";

    wxFile dataFile(filename, wxFile::write);

    if (dataFile.IsOpened())
    {
        dataFile.Write(data.c_str(), data.size());
    }
    else
    {
        result = false;
        msg = "Unable to create file " + filename;
    }

    return result;
}

bool ScheduleManager::RetrieveData(const std::string& key, std::string& data, std::string& msg) const
{
    bool result = true;

    std::string filename = _showDir + "/xScheduleData/" + key + ".dat";

    if (!wxFile::Exists(filename))
    {
        data = "";
        result = false;
    }
    else
    {
        wxFile dataFile(filename);

        wxString d = "";
        dataFile.ReadAll(&d);
        data = d.ToStdString();
    }

    return result;
}

bool ScheduleManager::ToggleOutputToLights(std::string& msg)
{
    bool result;
    if (_outputManager->IsOutputting())
    {
        _outputManager->StopOutput();
    }
    else
    {
        result = _outputManager->StartOutput();
        if (!result)
        {
            msg = "Problem starting light output.";
            return false;
        }
    }

    return true;
}

bool ScheduleManager::ToggleCurrentPlayListRandom(std::string& msg)
{
    PlayList* p = GetRunningPlayList();

    if (p != nullptr)
    {
        return p->SetRandom(!p->IsRandom());
    }
    else
    {
        msg = "No playlist currently playing.";
        return false;
    }
}

bool ScheduleManager::ToggleCurrentPlayListPause(std::string& msg)
{
    PlayList* p = GetRunningPlayList();
    if (p != nullptr)
    {
        p->Pause();
    }
    else
    {
        msg = "No playlist currently playing.";
        return false;
    }

    return true;
}

bool ScheduleManager::ToggleCurrentPlayListLoop(std::string& msg)
{
    PlayList* p = GetRunningPlayList();

    if (p != nullptr)
    {
        return p->SetLooping(!p->IsLooping());
    }
    else
    {
        msg = "No playlist currently playing.";
        return false;
    }
}

bool ScheduleManager::ToggleCurrentPlayListStepLoop(std::string& msg)
{
    PlayList* p = GetRunningPlayList();

    if (p != nullptr)
    {
        if (p->IsStepLooping())
        {
            p->ClearStepLooping();
        }
        else
        {
            if (!p->LoopStep(p->GetRunningStep()->GetNameNoTime()))
            {
                msg = "Unable to loop the current step.";
                return false;
            }
        }
        return true;
    }
    else
    {
        msg = "No playlist currently playing.";
        return false;
    }
}

bool ScheduleManager::IsOutputToLights() const
{
    return _outputManager != nullptr && _outputManager->IsOutputting();
}