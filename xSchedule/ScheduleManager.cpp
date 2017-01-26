#include "ScheduleManager.h"
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include "ScheduleOptions.h"
#include "PlayList/PlayList.h"
#include "../xLights/outputs/OutputManager.h"
#include "PlayList/PlayListStep.h"
#include "RunningSchedule.h"
#include <log4cpp/Category.hh>
#include <wx/dir.h>
#include <wx/file.h>
#include "../xLights/xLightsVersion.h"
#include "../xLights/AudioManager.h"
#include "xScheduleMain.h"
#include "xScheduleApp.h"
#include <wx/config.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "UserButton.h"
#include "FSEQFile.h"
#include "OutputProcess.h"

ScheduleManager::ScheduleManager(const std::string& showDir)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Loading schedule from %s.", (const char *)_showDir.c_str());

    // prime fix file with our show directory for any filename fixups
    FSEQFile::FixFile(showDir, "");

    _queuedSongs = new PlayList();
    _fppSync = nullptr;
    _manualOTL = -1;
    _immediatePlay = nullptr;
    _scheduleOptions = nullptr;
    _showDir = showDir;
    _startTime = wxGetUTCTimeMillis().GetLo();
    _outputManager = nullptr;
    _buffer = nullptr;
    _brightness = 100;
    _lastBrightness = 100;

    wxConfigBase* config = wxConfigBase::Get();
    _mode = (SYNCMODE)config->ReadLong("SyncMode", SYNCMODE::STANDALONE);

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    _lastSavedChangeCount = 0;
    _changeCount = 0;
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
            else if (n->GetName() == "OutputProcesses")
            {
                for (wxXmlNode* n1 = n->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
                {
                    OutputProcess* op = OutputProcess::CreateFromXml(n1);
                    if (op != nullptr)
                    {
                        _outputProcessing.push_back(op);
                    }
                }
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

void ScheduleManager::AddPlayList(PlayList* playlist)
{
    _playLists.push_back(playlist);     
    _changeCount++;
}

std::list<PlayList*> ScheduleManager::GetPlayLists()
{
    return _playLists;
}

ScheduleManager::~ScheduleManager()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    CloseFPPSyncSendSocket();
    _outputManager->StopOutput();
    logger_base.info("Stopped outputting to lights.");
    if (IsDirty())
	{
		if (wxMessageBox("Unsaved changes to the schedule. Save now?", "Unsave changes", wxYES_NO) == wxID_YES)
		{
			Save();
		}
	}

    while (_outputProcessing.size() > 0)
    {
        auto toremove = _outputProcessing.front();
        _outputProcessing.remove(toremove);
        delete toremove;
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

    if (_queuedSongs != nullptr)
    {
        delete _queuedSongs;
        _queuedSongs = nullptr;
    }

    while (_activeSchedules.size() > 0)
    {
        auto toremove = _activeSchedules.front();
        _activeSchedules.remove(toremove);
        delete toremove;
    }

    delete _scheduleOptions;
    delete _outputManager;
    free(_buffer);

    logger_base.info("Closed schedule.");
}

bool ScheduleManager::IsDirty()
{
    bool res = _lastSavedChangeCount != _changeCount;

    auto it = _playLists.begin();
    while (!res && it != _playLists.end())
    {
        res = res || (*it)->IsDirty();
        ++it;
    }

    res = res || _scheduleOptions->IsDirty();

    for (auto it = _outputProcessing.begin(); it != _outputProcessing.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

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

    if (_outputProcessing.size() != 0)
    {
        wxXmlNode* op = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OutputProcesses");

        for (auto it = _outputProcessing.begin(); it != _outputProcessing.end(); ++it)
        {
            op->AddChild((*it)->Save());
        }

        root->AddChild(op);
    }

    doc.Save(_showDir + "/" + GetScheduleFile());
    ClearDirty();
    logger_base.info("Saved Schedule to %s.", (const char*)(_showDir + "/" + GetScheduleFile()).c_str());
}

void ScheduleManager::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    _scheduleOptions->ClearDirty();

    for (auto it = _outputProcessing.begin(); it != _outputProcessing.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}

void ScheduleManager::RemovePlayList(PlayList* playlist)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Deleting playlist %s.", (const char*)playlist->GetNameNoTime().c_str());
    _playLists.remove(playlist);
    _changeCount++;
}

PlayList* ScheduleManager::GetRunningPlayList() const
{
    // find the highest priority running playlist
    PlayList* running = nullptr;

    if (_immediatePlay != nullptr && _immediatePlay->IsRunning())
    {
        running = _immediatePlay;
    }
    else if (_queuedSongs->GetSteps().size() > 0 && _queuedSongs->IsRunning())
    {
        running = _queuedSongs;
    }
    else
    {
        if (GetRunningSchedule() != nullptr)
        {
            running = GetRunningSchedule()->GetPlayList();
        }
    }

    return running;
}

void ScheduleManager::StopAll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Stopping all playlists.");

    if (_immediatePlay != nullptr)
    {
        _immediatePlay->Stop();
        delete _immediatePlay;
        _immediatePlay = nullptr;
    }

    if (_queuedSongs->IsRunning())
    {
        _queuedSongs->Stop();
        _queuedSongs->RemoveAllSteps();
    }

    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        (*it)->GetPlayList()->Stop();
    }
}

void ScheduleManager::Frame(bool outputframe)
{
    PlayList* running = GetRunningPlayList();

    if (running != nullptr)
    {
        long msec = wxGetUTCTimeMillis().GetLo() - _startTime;
        if (outputframe)
        {
            memset(_buffer, 0x00, _outputManager->GetTotalChannels()); // clear out any prior frame data
            _outputManager->StartFrame(msec);
        }
        bool done = running->Frame(_buffer, _outputManager->GetTotalChannels(), outputframe);
        if (outputframe)
        {
            // apply any output processing
            for (auto it = _outputProcessing.begin(); it != _outputProcessing.end(); ++it)
            {
                (*it)->Frame(_buffer, _outputManager->GetTotalChannels());
            }

            _outputManager->SetManyChannels(0, _buffer, _outputManager->GetTotalChannels());
            _outputManager->EndFrame();
        }

        if (done)
        {
            // playlist is done
            StopPlayList(running, false);
            wxCommandEvent event(EVT_SCHEDULECHANGED);
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }

        if (outputframe && _mode == SYNCMODE::FPPMASTER)
        {
            SendFPPSync(running->GetActiveSyncItemName(), msec);
        }
    }
    else
    {
        if (_scheduleOptions->IsSendOffWhenNotRunning())
        {
            _outputManager->StartFrame(0);
            _outputManager->AllOff();
            // apply any output processing
            for (auto it = _outputProcessing.begin(); it != _outputProcessing.end(); ++it)
            {
                (*it)->Frame(_buffer, _outputManager->GetTotalChannels());
            }
            _outputManager->EndFrame();
        }
    }

    if (outputframe && _brightness < 100)
    {
        if (_brightness != _lastBrightness)
        {
            _lastBrightness = _brightness;
            CreateBrightnessArray();
        }

        wxByte* pb = _buffer;
        for (int i = 0; i < _outputManager->GetTotalChannels(); ++i)
        {
            *pb = _brightnessArray[*pb];
            pb++;
        }
    }
}

void ScheduleManager::CreateBrightnessArray()
{
    for (int i = 0; i < 255; i++)
    {
        _brightnessArray[i] = ((255 * _brightness) / 100) & 0xFF;
    }
}

bool ScheduleManager::PlayPlayList(PlayList* playlist, size_t& rate, bool loop, const std::string& step, bool forcelast, int plloops, bool random, int steploops)
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

    if (_queuedSongs->IsRunning())
    {
        logger_base.info("Suspending queued playlist so immediate can play.");
        _queuedSongs->Suspend(true);
    }

    // this needs to create a copy of everything ... including steps etc
    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if (!(*it)->GetPlayList()->IsSuspended())
        {
            logger_base.info("Suspending playlist %s due to schedule %s so immediate can play.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
            (*it)->GetPlayList()->Suspend(true);
        }
    }

    _immediatePlay = new PlayList(*playlist);
    _immediatePlay->Start(loop, random, plloops);
    if (step != "")
    {
        _immediatePlay->JumpToStep(step);
        _immediatePlay->GetRunningStep()->SetLoops(steploops);
    }

    if (forcelast)
    {
        _immediatePlay->StopAtEndOfCurrentStep();
    }

    rate = 25; // always start fast
    return result;
}

bool compare_runningschedules(const RunningSchedule* first, const RunningSchedule* second)
{
    return first->GetSchedule()->GetPriority() > second->GetSchedule()->GetPriority();
}

int ScheduleManager::CheckSchedule()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Checking the schedule ...");

    // check all the schedules and add into the list any that should be in the active schedules list
    for (auto it = _playLists.begin(); it != _playLists.end(); ++it)
    {
        auto schedules = (*it)->GetSchedules();
        for (auto it2 = schedules.begin(); it2 != schedules.end(); ++it2)
        {
            logger_base.debug("   Checking playlist %s schedule %s.", (const char *)(*it)->GetNameNoTime().c_str(), (const char *)(*it2)->GetName().c_str());
            if ((*it2)->CheckActive())
            {
                logger_base.debug("   It should be active.");
                bool found = false;

                for (auto it3 = _activeSchedules.begin(); it3 != _activeSchedules.end(); ++it3)
                {
                    if ((*it3)->GetSchedule()->GetId() == (*it2)->GetId())
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    // is hasnt been active before now
                    RunningSchedule* rs = new RunningSchedule(*it, *it2);
                    _activeSchedules.push_back(rs);
                    rs->GetPlayList()->StartSuspended(rs->GetSchedule()->GetLoop(), rs->GetSchedule()->GetRandom(), rs->GetSchedule()->GetLoops());

                    logger_base.info("   Scheduler starting suspended playlist %s due to schedule %s.", (const char*)(*it)->GetNameNoTime().c_str(),  (const char *)(*it2)->GetName().c_str());
                }
                else
                {
                    logger_base.debug("   It was already in the list.");
                }
            }
        }
    }

    std::list<RunningSchedule*> todelete;
    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if (!(*it)->GetSchedule()->CheckActive())
        {

            if (!(*it)->GetPlayList()->IsRunning())
            {
                logger_base.info("   Scheduler removing playlist %s due to schedule %s.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                // this shouldnt be in the list any longer
                todelete.push_back(*it);
            }
            else
            {
                if (!(*it)->GetPlayList()->IsFinishingUp())
                {
                    logger_base.info("   Scheduler telling playlist %s due to schedule %s it is time to finish up.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                    (*it)->GetPlayList()->JumpToEndStepsAtEndOfCurrentStep();
                }
            }
        }
    }

    for (auto it = todelete.begin(); it != todelete.end(); ++it)
    {
        _activeSchedules.remove(*it);
        delete *it;
    }

    int framems = 50;

    _activeSchedules.sort(compare_runningschedules);

    if (_immediatePlay == nullptr)
    {
        if (_queuedSongs->GetSteps().size() == 0)
        {
            bool first = true;
            for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
            {
                if (first)
                {
                    if ((*it)->GetPlayList()->IsRunning())
                    {
                        first = false;
                        if ((*it)->GetPlayList()->IsSuspended())
                        {
                            logger_base.info("   Unsuspending playlist %s due to schedule %s.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                            framems = (*it)->GetPlayList()->Suspend(false);
                        }
                    }
                }
                else
                {
                    if (!(*it)->GetPlayList()->IsSuspended())
                    {
                        logger_base.info("   Suspending playlist %s due to schedule %s.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                        (*it)->GetPlayList()->Suspend(true);
                    }
                }
            }
        }
        else
        {
            // we should be playing our queued song
            // make sure they are all suspended
            for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
            {
                if (!(*it)->GetPlayList()->IsSuspended())
                {
                    logger_base.info("   Suspending playlist %s due to schedule %s so immediate can play.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                    (*it)->GetPlayList()->Suspend(true);
                }
            }

            if (!_queuedSongs->IsRunning())
            {
                // we need to start it
                _queuedSongs->Start();
            }
            else if  (_queuedSongs->IsSuspended())
            {
                // we need to unsuspend it
                _queuedSongs->Suspend(false);
            }
            else
            {
                // it is already running
            }

            framems = _queuedSongs->GetRunningStep()->GetFrameMS();
        }
    }
    else
    {
        // make sure they are all suspended
        for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
        {
            if (!(*it)->GetPlayList()->IsSuspended())
            {
                logger_base.info("   Suspending playlist %s due to schedule %s so immediate can play.", (const char*)(*it)->GetPlayList()->GetNameNoTime().c_str(), (const char *)(*it)->GetSchedule()->GetName().c_str());
                (*it)->GetPlayList()->Suspend(true);
            }
        }

        if (_queuedSongs->GetSteps().size() > 0 && !_queuedSongs->IsSuspended())
        {
            logger_base.info("   Suspending queued playlist so immediate can play.");
            _queuedSongs->Suspend(true);
        }

        framems = _immediatePlay->GetRunningStep()->GetFrameMS();
    }

    logger_base.debug("   Active scheduled playlists: %d", _activeSchedules.size());

    return framems;
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

bool ScheduleManager::IsQueuedPlaylistRunning() const
{
    return GetRunningPlayList() != nullptr && _queuedSongs->GetId() == GetRunningPlayList()->GetId();
}

// localhost/xScheduleCommand?Command=<command>&Parameters=<comma separated parameters>
bool ScheduleManager::Action(const std::string command, const std::string parameters, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg)
{
    bool result = true;

    Command* cmd = _commandManager.GetCommand(command);

    if (cmd == nullptr)
    {
        result = false;
        msg = "Unknown command.";
    }
    else
    {
        if (!cmd->IsValid(parameters, selplaylist, selschedule, this, msg, IsQueuedPlaylistRunning()))
        {
            result = false;
        }
        else
        {
            if (command == "Stop all now")
            {
                StopAll();
            }
            else if (command == "Stop")
            {
                PlayList* p = GetRunningPlayList();
                if (p != nullptr)
                {
                    p->Stop();

                    if (_immediatePlay != nullptr && p->GetId() == _immediatePlay->GetId())
                    {
                        delete _immediatePlay;
                        _immediatePlay = nullptr;
                    }
                    else if (p->GetId() == _queuedSongs->GetId())
                    {
                        p->RemoveAllSteps();
                    }
                }
            }
            else if (command == "Play selected playlist")
            {
                if (selplaylist != nullptr)
                {
                    if (!PlayPlayList(selplaylist, rate))
                    {
                        result = false;
                        msg = "Unable to start playlist.";
                    }
                }
            }
            else if (command == "Play selected playlist looped")
            {
                if (selplaylist != nullptr)
                {
                    if (!PlayPlayList(selplaylist, rate, true))
                    {
                        result = false;
                        msg = "Unable to start playlist.";
                    }
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
            }
            else if (command == "Stop specified playlist")
            {
                PlayList* p = GetPlayList(parameters);

                if (p != nullptr)
                {
                    StopPlayList(p, false);
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
            }
            else if (command == "Stop specified playlist at end of current loop")
            {
                PlayList* p = GetPlayList(parameters);

                if (p != nullptr)
                {
                    p->StopAtEndOfThisLoop();
                }
            }
            else if (command == "Jump to play once at end at end of current step and then stop")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    p->JumpToEndStepsAtEndOfCurrentStep();
                }
            }
            else if (command == "Stop playlist at end of current loop")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    p->StopAtEndOfThisLoop();
                }
            }
            else if (command == "Pause")
            {
                result = ToggleCurrentPlayListPause(msg);
            }
            else if (command == "Next step in current playlist")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr && IsQueuedPlaylistRunning())
                {
                    if (p->GetId() == _queuedSongs->GetId())
                    {
                        _queuedSongs->Stop();
                        _queuedSongs->RemoveStep(_queuedSongs->GetSteps().front());
                        _queuedSongs->Start();
                    }
                    else
                    {
                        rate = p->JumpToNextStep();
                    }
                }
            }
            else if (command == "Restart step in current playlist")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    p->RestartCurrentStep();
                }
            }
            else if (command == "Prior step in current playlist")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    rate = p->JumpToPriorStep();
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
            }
            else if (command == "Play specified playlist step once only")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

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
            }
            else if (command == "Enqueue playlist step")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

                std::string pl = split[0].ToStdString();
                std::string step = split[1].ToStdString();

                PlayList* p = GetPlayList(pl);

                if (p != nullptr)
                {
                    PlayListStep* pls = p->GetStep(step);

                    if (pls != nullptr)
                    {
                        if (_queuedSongs->GetSteps().size() > 1 && _queuedSongs->GetSteps().back()->GetId() != pls->GetId())
                        {
                            _queuedSongs->AddStep(new PlayListStep(*pls), -1);
                            if (!_queuedSongs->IsRunning())
                            {
                                _queuedSongs->StartSuspended();
                            }
                        }
                        else
                        {
                            result = false;
                            msg = "step is already at the end of the list ... I wont add a duplicate.";
                        }
                    }
                    else
                    {
                        result = false;
                        msg = "Unknown step.";
                    }
                }
            }
            else if (command == "Clear playlist queue")
            {
                if (_queuedSongs->IsRunning())
                {
                    _queuedSongs->Stop();
                }

                _queuedSongs->RemoveAllSteps();
            }
            else if (command == "Play playlist starting at step")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

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
            }
            else if (command == "Play playlist starting at step looped")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

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
            }
            else if (command == "Play playlist step")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

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
            }
            else if (command == "Jump to specified step in current playlist")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    rate = p->JumpToStep(parameters);
                }
            }
            else if (command == "Jump to specified step in current playlist at the end of current step")
            {
                PlayList* p = GetRunningPlayList();

                if (p != nullptr)
                {
                    p->JumpToStepAtEndOfCurrentStep(parameters);
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
            }
            else if (command == "Add to the current schedule n minutes")
            {
                RunningSchedule *rs = GetRunningSchedule();
                if (rs != nullptr && rs->GetSchedule() != nullptr)
                {
                    rs->GetSchedule()->AddMinsToEndTime(wxAtoi(parameters));
                }
            }
            else if (command == "Set volume to")
            {
                int volume = wxAtoi(parameters);
                SetVolume(volume);
            }
            else if (command == "Adjust volume by")
            {
                int volume = wxAtoi(parameters);
                AdjustVolumeBy(volume);
            }
            else if (command == "Toggle output to lights")
            {
                result = ToggleOutputToLights(msg);
            }
            else if (command == "Toggle mute")
            {
                ToggleMute();
            }
            else if (command == "Increase brightness by n%")
            {
                int by = wxAtoi(parameters);
                AdjustBrightness(by);
            }
            else if (command ==  "Set brightness to n%")
            {
                int b = wxAtoi(parameters);
                SetBrightness(b);
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
            else if (command == "Restart selected schedule")
            {
                auto rs = GetRunningSchedule();
                if (rs != nullptr)
                    rs->Reset();
            }
            else if (command == "Restart named schedule")
            {
                auto rs = GetRunningSchedule(parameters);
                if (rs != nullptr)
                    rs->Reset();
            }
            else if (command == "PressButton")
            {
                UserButton* b = _scheduleOptions->GetButton(parameters);

                if (b != nullptr)
                {
                    std::string c = b->GetCommand();
                    std::string p = b->GetParameters();

                    if (c != "")
                    {
                        result = Action(c, p, selplaylist, selschedule, rate, msg);
                    }
                }
                else
                {
                    result = false;
                    msg = "Unknown button.";
                }
            }
            else if (command == "Play specified playlist n times")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

                std::string pl = split[0].ToStdString();
                int loops = wxAtoi(split[1]);
                PlayList* p = GetPlayList(pl);

                if (p != nullptr)
                {
                    if (!PlayPlayList(p, rate, false, "", false, loops))
                    {
                        result = false;
                        msg = "Unable to start playlist.";
                    }
                }
            }
            else if (command == "Play specified playlist step n times")
            {
                wxString parameter = parameters;
                wxArrayString split = wxSplit(parameter, ',');

                std::string pl = split[0].ToStdString();
                std::string step = split[1].ToStdString();
                int loops = wxAtoi(split[2]);

                PlayList* p = GetPlayList(pl);

                if (p != nullptr)
                {
                    if (!PlayPlayList(p, rate, false, step, false, -1, false, loops))
                    {
                        result = false;
                        msg = "Unable to start playlist.";
                    }
                }
            }
        }
    }
 
    if (!result)
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("Action failed: %s", (const char *)msg.c_str());

        wxCommandEvent event(EVT_STATUSMSG);
        event.SetString(msg);
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }

    // Clean up immediate play of one of the actions led to it stopping
    if (_immediatePlay != nullptr)
    {
        if (!_immediatePlay->IsRunning())
        {
            delete _immediatePlay;
            _immediatePlay = nullptr;
        }
    }

    wxCommandEvent event(EVT_SCHEDULECHANGED);
    wxPostEvent(wxGetApp().GetTopWindow(), event);

    return result;
}


bool ScheduleManager::Action(const std::string label, PlayList* selplaylist, Schedule* selschedule, size_t& rate, std::string& msg)
{
    UserButton* b = _scheduleOptions->GetButton(label);

    if (b != nullptr)
    {
        std::string command = b->GetCommand();
        std::string parameters = b->GetParameters();

        return Action(command, parameters, selplaylist, selschedule, rate, msg);
    }
    else
    {
        msg = "Unknown button.";
        return false;
    }
}

void ScheduleManager::StopPlayList(PlayList* playlist, bool atendofcurrentstep)
{
    if (_immediatePlay != nullptr && _immediatePlay->GetId() == playlist->GetId())
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
        if ((*it)->GetId() == playlist->GetId() && (*it)->IsRunning())
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
            data += "{\"name\":\"" + (*it)->GetNameNoTime() + 
                    "\",\"id\":\"" + wxString::Format(wxT("%i"), (*it)->GetId()).ToStdString() + 
                    "\",\"length\":\""+ FormatTime((*it)->GetLengthMS()) +"\"}";
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
                data += "{\"name\":\"" + (*it)->GetNameNoTime() + 
                        "\",\"id\":\"" + wxString::Format(wxT("%i"), (*it)->GetId()).ToStdString() + 
                        "\",\"length\":\""+FormatTime((*it)->GetLengthMS())+"\"}";
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
    else if (command == "GetQueuedSteps")
    {
        PlayList* p = _queuedSongs;

        data = "{\"steps\":[";
        auto steps = p->GetSteps();
        for (auto it = steps.begin(); it != steps.end(); ++it)
        {
            if (it != steps.begin())
            {
                data += ",";
            }
            data += "{\"name\":\"" + (*it)->GetNameNoTime() + 
                    "\",\"id\":\"" + wxString::Format(wxT("%i"), (*it)->GetId()).ToStdString() + 
                    "\",\"length\":\"" + FormatTime((*it)->GetLengthMS()) + "\"}";
        }
        data += "]}";
    }
    else if (command == "GetPlayingStatus")
    {
        PlayList* p = GetRunningPlayList();
        if (p == nullptr || p->GetRunningStep() == nullptr)
        {
            data = "{\"status\":\"idle\",\"outputtolights\":\"" + std::string(_outputManager->IsOutputting() ? "true" : "false") + 
                "\",\"volume\":\"" + wxString::Format(wxT("%i"), GetVolume()) +
                "\",\"time\":\""+ wxDateTime::Now().Format("%Y-%m-%d %H:%M:%S") +"\"}";
        }
        else
        {
            std::string nextsong;
            bool didloop;
            auto next = p->GetNextStep(didloop);
            if (next == nullptr)
            {
                nextsong = "";
            }
            else if (p->IsRandom())
            {
                nextsong = "God knows";
            }
            else
            {
                nextsong = next->GetNameNoTime();
            }

            data = "{\"status\":\"" + std::string(p->IsPaused() ? "paused" : "playing") + 
                "\",\"playlist\":\"" + p->GetNameNoTime() +
                "\",\"playlistid\":\"" + wxString::Format(wxT("%i"), p->GetId()).ToStdString() +
                "\",\"playlistlooping\":\"" + (p->IsLooping() || p->GetLoopsLeft() > 0 ? "true" : "false") +
                "\",\"playlistloopsleft\":\"" + wxString::Format(wxT("%i"),p->GetLoopsLeft()).ToStdString() +
                "\",\"random\":\"" + (p->IsRandom() ? "true" : "false") +
                "\",\"step\":\"" + p->GetRunningStep()->GetNameNoTime() +
                "\",\"stepid\":\"" + wxString::Format(wxT("%i"), p->GetRunningStep()->GetId()).ToStdString() +
                "\",\"steplooping\":\"" + (p->IsStepLooping() || p->GetRunningStep()->GetLoopsLeft() > 0 ? "true" : "false") +
                "\",\"steploopsleft\":\"" + wxString::Format(wxT("%i"), p->GetRunningStep()->GetLoopsLeft()).ToStdString() +
                "\",\"length\":\"" + FormatTime(p->GetRunningStep()->GetLengthMS()) +
                "\",\"position\":\"" + FormatTime(p->GetRunningStep()->GetPosition()) +
                "\",\"left\":\"" + FormatTime(p->GetRunningStep()->GetLengthMS() - p->GetRunningStep()->GetPosition()) + 
                "\",\"trigger\":\"" + std::string(IsCurrentPlayListScheduled() ? "scheduled": (_immediatePlay != nullptr) ? "manual" : "queued") +
                "\",\"nextstep\":\"" + nextsong +
                "\",\"version\":\"" + xlights_version_string +
                "\",\"queuelength\":\"" + wxString::Format(wxT("%i"), _queuedSongs->GetSteps().size()) +
                "\",\"volume\":\"" + wxString::Format(wxT("%i"), GetVolume()) +
                "\",\"time\":\"" + wxDateTime::Now().Format("%Y-%m-%d %H:%M:%S") +
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

RunningSchedule* ScheduleManager::GetRunningSchedule() const
{
    if (_immediatePlay != nullptr) return nullptr;
    if (_queuedSongs->IsRunning()) return nullptr;
    if (_activeSchedules.size() == 0) return nullptr;

    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if ((*it)->GetPlayList()->IsRunning())
        {
            return *it;
        }
    }

    return nullptr;
}

bool ScheduleManager::IsScheduleActive(Schedule* schedule)
{
    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if ((*it)->GetSchedule()->GetId() == schedule->GetId()) return true;
    }

    return false;
}

RunningSchedule* ScheduleManager::GetRunningSchedule(Schedule* schedule) const
{
    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if ((*it)->GetSchedule()->GetId() == schedule->GetId()) return *it;
    }

    return nullptr;
}

RunningSchedule* ScheduleManager::GetRunningSchedule(const std::string& schedulename) const
{
    for (auto it = _activeSchedules.begin(); it != _activeSchedules.end(); ++it)
    {
        if ((*it)->GetSchedule()->GetName() == schedulename) return *it;
    }

    return nullptr;
}

void ScheduleManager::SetOutputToLights(bool otl)
{
    if (_outputManager != nullptr)
    {
        if (otl)
        {
            if (!IsOutputToLights())
            {
                _outputManager->StartOutput();
            }
        }
        else
        {
            if (IsOutputToLights())
            {
                _outputManager->StopOutput();
            }
        }
    }
}

void ScheduleManager::ManualOutputToLightsClick()
{
    _manualOTL++;
    if (_manualOTL > 1) _manualOTL = -1;
    if (_manualOTL == 1)
    {
        _outputManager->StartOutput();
    }
    else if (_manualOTL == 0)
    {
        _outputManager->StopOutput();
    }
}

bool ScheduleManager::ToggleOutputToLights(std::string& msg)
{
    if (_outputManager->IsOutputting())
    {
        _manualOTL = 0;
        SetOutputToLights(false);
    }
    else
    {
        _manualOTL = 1;
        SetOutputToLights(true);
    }

    return true;
}

void ScheduleManager::SetVolume(int volume)
{
    int cv = volume;
    if (cv < 0) cv = 0;
    if (cv > 100) cv = 100;
    AudioManager::SetGlobalVolume(cv);
}
void ScheduleManager::AdjustVolumeBy(int volume)
{
    int cv = GetVolume();
    cv += volume;
    SetVolume(cv);
}
int ScheduleManager::GetVolume() const
{
    return AudioManager::GetGlobalVolume();
}

size_t ScheduleManager::GetTotalChannels() const
{
    if (_outputManager != nullptr)
        return _outputManager->GetTotalChannels();

    return 0;
}

void ScheduleManager::ToggleMute()
{
    static int savevolume = 100;
    if (GetVolume() > 0)
    {
        savevolume = GetVolume();
        SetVolume(0);
    }
    else
    {
        SetVolume(savevolume);
    }
}

void ScheduleManager::SetMode(SYNCMODE mode)
{
    if (_mode != mode)
    {
        _mode = mode;

        wxConfigBase* config = wxConfigBase::Get();
        config->Write("SyncMode", (long)_mode);

        if (_mode == SYNCMODE::FPPMASTER)
        {
            OpenFPPSyncSendSocket();
        }
        else
        {
            CloseFPPSyncSendSocket();
        }
    }
}

void ScheduleManager::SendFPPSync(const std::string& syncItem, size_t msec)
{
    if (_fppSync == nullptr) return;
    if (syncItem == "") return;
    
#pragma todo
}

void ScheduleManager::OpenFPPSyncSendSocket()
{
    CloseFPPSyncSendSocket();

//    wxIPV4address address;
//    address.Hostname(ip);
//    address.Service(4352);

//    _fppSync = new wxSocketClient();

#pragma todo
}

void ScheduleManager::CloseFPPSyncSendSocket()
{
    if (_fppSync != nullptr) {
        _fppSync->Close();
        delete _fppSync;
        _fppSync = nullptr;
    }
}
