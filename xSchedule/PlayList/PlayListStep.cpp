/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListStep.h"

#include "PlayList.h"
#include "PlayListItemVideo.h"
#include "PlayListItemImage.h"
#include "PlayListItemJukebox.h"
#include "PlayListItemESEQ.h"
#include "PlayListItemFade.h"
#include "PlayListItemFile.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemText.h"
#include "PlayListItemScreenMap.h"
#include "PlayListItemFSEQVideo.h"
#include "PlayListItemTest.h"
#include "PlayListItemMicrophone.h"
#include "PlayListItemColourOrgan.h"
#include "PlayListItemRDS.h"
#include "PlayListItemProjector.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemSetColour.h"
#include "PlayListItemDelay.h"
#include "PlayListItemDim.h"
#include "PlayListItemRunProcess.h"
#include "PlayListItemCURL.h"
#include "PlayListItemSerial.h"
#include "PlayListItemMIDI.h"
#include "PlayListItemFPPEvent.h"
#include "PlayListItemRunCommand.h"
#include "PlayListItemMQTT.h"
#include "PlayListItemOSC.h"
#include "PlayListItemAudio.h"
#include "PlayListItemARTNetTrigger.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ReentrancyCounter.h"
#include "../../xLights/UtilFunctions.h"

#include <wx/filename.h>
#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>

int __playliststepid = 0;

PlayListStep::PlayListStep(OutputManager* outputManager, wxXmlNode* node)
{
    _reentrancyCounter = 0;
    _loops = -1;
    _pause = 0;
    _suspend = 0;
    _id = __playliststepid++;
    _startTime = 0;
    _framecount = 0;
    _excludeFromRandom = false;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _everyStep = false;
    Load(outputManager, node);
}

bool compare_priority(const PlayListItem* first, const PlayListItem* second)
{
    return first->GetPriority() > second->GetPriority();
}

PlayListStep::PlayListStep()
{
    _reentrancyCounter = 0;
    _loops = -1;
    _pause = 0;
    _suspend = 0;
    _id = __playliststepid++;
    _startTime = 0;
    _framecount = 0;
    _name = "";
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _excludeFromRandom = false;
    _everyStep = false;
}

PlayListStep::PlayListStep(const PlayListStep& step)
{
    _reentrancyCounter = 0;
    _startTime = 0;
    _pause = 0;
    _suspend = 0;
    _loops = step._loops;
    _framecount = step._framecount;
    _name = step._name;
    _lastSavedChangeCount = step._lastSavedChangeCount;
    _changeCount = step._changeCount;
    _excludeFromRandom = step._excludeFromRandom;
    _everyStep = step._everyStep;
    _id = step._id;
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it = step._items.begin(); it != step._items.end(); ++it)
        {
            _items.push_back((*it)->Copy());
        }
        _items.sort(compare_priority);
    }
}

void PlayListStep::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            (*it)->ClearDirty();
        }
    }
}

PlayListStep::~PlayListStep()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    {
        ReentrancyCounter rec(_reentrancyCounter);

        if (!rec.SoleReference())
        {
            logger_base.warn("PlayListStep being destroyed but we appear to be manipulating it elsewhere. This may not end well.");
        }

        while (_items.size() > 0)
        {
            auto toremove = _items.front();
            _items.remove(toremove);
            delete toremove;
        }
    }
}

wxXmlNode* PlayListStep::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PlayListStep");

    res->AddAttribute("Name", _name);
    if (_excludeFromRandom)
    {
        res->AddAttribute("ExcludeRandom", "TRUE");
    }
    if (_everyStep)
    {
        res->AddAttribute("EveryStep", "TRUE");
    }

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            res->AddChild((*it)->Save());
        }
    }

    return res;
}

void PlayListStep::Load(OutputManager* outputManager, wxXmlNode* node)
{
    _name = node->GetAttribute("Name", "");
    _excludeFromRandom = node->GetAttribute("ExcludeRandom", "FALSE") == "TRUE";
    _everyStep = node->GetAttribute("EveryStep", "FALSE") == "TRUE";

    for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "PLIVideo")
        {
            _items.push_back(new PlayListItemVideo(n));
        }
        else if (n->GetName() == "PLIFSEQ")
        {
            _items.push_back(new PlayListItemFSEQ(outputManager, n));
        }
        else if (n->GetName() == "PLIFile")
        {
            _items.push_back(new PlayListItemFile(n));
        }
        else if (n->GetName() == "PLIText")
        {
            _items.push_back(new PlayListItemText(n));
        }
        else if (n->GetName() == "PLIScreenMap")
        {
            _items.push_back(new PlayListItemScreenMap(n));
        }
        else if (n->GetName() == "PLIAudio")
        {
            _items.push_back(new PlayListItemAudio(n));
        }
        else if (n->GetName() == "PLIARTNetTrigger")
        {
            _items.push_back(new PlayListItemARTNetTrigger(n));
        }
        else if (n->GetName() == "PLIFSEQVideo")
        {
            _items.push_back(new PlayListItemFSEQVideo(outputManager, n));
        }
        else if (n->GetName() == "PLITest")
        {
            _items.push_back(new PlayListItemTest(outputManager, n));
        }
        else if (n->GetName() == "PLIMicrophone")
        {
            _items.push_back(new PlayListItemMicrophone(outputManager, n));
        }
        else if (n->GetName() == "PLIColourOrgan")
        {
            _items.push_back(new PlayListItemColourOrgan(outputManager, n));
        }
        else if (n->GetName() == "PLIRDS")
        {
            _items.push_back(new PlayListItemRDS(n));
        }
        else if (n->GetName() == "PLIPJLink")
        {
            wxMessageBox("PJLink playlist item has been removed. Please re-create these using projector playlist item.");
            _items.push_back(new PlayListItemProjector());
        }
        else if (n->GetName() == "PLIProjector")
        {
            _items.push_back(new PlayListItemProjector(n));
        }
        else if (n->GetName() == "PLIESEQ")
        {
            _items.push_back(new PlayListItemESEQ(n));
        }
        else if (n->GetName() == "PLIFade")
        {
            _items.push_back(new PlayListItemFade(outputManager, n));
        }
        else if (n->GetName() == "PLIImage")
        {
            _items.push_back(new PlayListItemImage(n));
        }
        else if (n->GetName() == "PLIJukebox")
        {
            _items.push_back(new PlayListItemJukebox(n));
        }
        else if (n->GetName() == "PLIAllSet" || n->GetName() == "PLIAllOff")
        {
            _items.push_back(new PlayListItemAllOff(outputManager, n));
        }
        else if (n->GetName() == "PLISetColour")
        {
            _items.push_back(new PlayListItemSetColour(outputManager, n));
        }
        else if (n->GetName() == "PLIDelay")
        {
            _items.push_back(new PlayListItemDelay(n));
        }
        else if (n->GetName() == "PLIDim")
        {
            _items.push_back(new PlayListItemDim(outputManager, n));
        }
        else if (n->GetName() == "PLIProcess")
        {
            _items.push_back(new PlayListItemRunProcess(n));
        }
        else if (n->GetName() == "PLICURL")
        {
            _items.push_back(new PlayListItemCURL(n));
        }
        else if (n->GetName() == "PLISERIAL")
        {
            _items.push_back(new PlayListItemSerial(n));
        }
        else if (n->GetName() == "PLIMIDI")
        {
            _items.push_back(new PlayListItemMIDI(n));
        }
        else if (n->GetName() == "PLIFPPEVENT")
        {
            _items.push_back(new PlayListItemFPPEvent(n));
        }
        else if (n->GetName() == "PLICommand")
        {
            _items.push_back(new PlayListItemRunCommand(n));
        }
        else if (n->GetName() == "PLIMQTT")
        {
            _items.push_back(new PlayListItemMQTT(n));
        }
        else if (n->GetName() == "PLIOSC")
        {
            _items.push_back(new PlayListItemOSC(n));
        }
        else if (n->GetName() == "PLIVirtualMatrix")
        {
            // ignore this node ... these are not supported here any more.
        }
        else
        {
            wxASSERT(false);
        }
    }

    _items.sort(compare_priority);

    for (const auto& it : _items)
    {
        it->SetStepLength(GetLengthMS());
    }
}

bool PlayListStep::IsDirty()
{
    bool res = _lastSavedChangeCount != _changeCount;

    {
        ReentrancyCounter rec(_reentrancyCounter);
        auto it = _items.begin();
        while (!res && it != _items.end())
        {
            res = res || (*it)->IsDirty();
            ++it;
        }
    }

    return res;
}

void PlayListStep::RemoveItem(PlayListItem* item)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    {
        ReentrancyCounter rec(_reentrancyCounter);

        if (!rec.SoleReference())
        {
            logger_base.warn("PlayListStep removing an item but we appear to be manipulating it elsewhere. This may not end well.");
        }

        _items.remove(item);
    }
    _changeCount++;
}

std::string PlayListStep::GetStartTime(PlayList* pl)
{
    if (pl == nullptr) return "";

    return pl->GetStepStartTime(this);
}

std::string PlayListStep::GetName(PlayList* pl)
{
    std::string offset = "";
    if (pl != nullptr)
    {
        offset = " {" + pl->GetStepStartTime(this) + "}";
    }

    std::string duration = "";
    if (GetLengthMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetLengthMS() / 1000.0).ToStdString() + "]";
    }

    if (_name != "") return _name + offset + duration;

    {
        ReentrancyCounter rec(_reentrancyCounter);
        if (_items.size() == 0) return "<unnamed>" + offset + duration;

        return _items.front()->GetNameNoTime() + offset + duration;
    }
}

std::string PlayListStep::GetNameNoTime()
{
    if (_name != "") return _name;

    {
        ReentrancyCounter rec(_reentrancyCounter);
        if (_items.size() == 0) return "<unnamed>";

        return _items.front()->GetNameNoTime();
    }
}
    
PlayListItem* PlayListStep::GetTimeSource(size_t &ms)
{
    ms = 9999;
    PlayListItem* timesource = nullptr;

    {
        ReentrancyCounter rec(_reentrancyCounter);
        // Prioritise the highest priotity FSEQ for timing
        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            if ((*it)->GetTitle() == "FSEQ" || (*it)->GetTitle() == "FSEQ & Video")
            {
                if ((*it)->GetDurationMS() > 0)
                {
                    size_t msec = (*it)->GetFrameMS();
                    if (timesource == nullptr)
                    {
                        timesource = *it;
                        ms = msec;
                    }
                    else
                    {
                        if (timesource != nullptr && (*it)->GetPriority() > timesource->GetPriority())
                        {
                            timesource = *it;
                            ms = msec;
                        }
                    }
                }
            }
        }

        if (timesource != nullptr)
        {
            if (ms == 9999)
            {
                ms = 50;
            }

            return timesource;
        }

        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            size_t msec = (*it)->GetFrameMS();

            if ((*it)->ControlsTiming())
            {
                if (timesource == nullptr && (*it)->GetDurationMS() > 0)
                {
                    timesource = *it;
                    ms = msec;
                }
                else
                {
                    if (timesource != nullptr && (*it)->GetDurationMS() > 0 && (*it)->GetPriority() > timesource->GetPriority())
                    {
                        timesource = *it;
                        ms = msec;
                    }
                }
            }
            else if (timesource == nullptr)
            {
                if (msec != 0 && msec < ms)
                {
                    ms = msec;
                }
            }
        }
    }

    if (ms == 9999)
    {
        ms = 50;
    }

    return timesource;
}

bool PlayListStep::Frame(uint8_t* buffer, size_t size, bool outputframe)
{
    ReentrancyCounter rec(_reentrancyCounter);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    size_t msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);

    if (msPerFrame == 0)
    {
        msPerFrame = 50;
    }

    size_t frameMS;
    if (timesource != nullptr)
    {
        frameMS = timesource->GetPositionMS();
    }
    else
    {
        frameMS = wxGetUTCTimeMillis().GetLo() - _startTime;
    }

    //logger_base.debug("Step %s frame %ld start.", (const char *)GetNameNoTime().c_str(), (long)frameMS);

    //if (frameMS >= GetLengthMS())
    //{
        // we are done
    //    return true;
    //}

    wxStopWatch sw;
    // we do this backwards to ensure the right render order
    for (auto it = _items.rbegin(); it != _items.rend(); ++it)
    {
        (*it)->Frame(buffer, size, frameMS, msPerFrame, outputframe);
    }

    if (sw.Time() > (float)msPerFrame * 0.8)
    {
        logger_base.warn("Step %s frame %ld took longer than 80%% frame time to output: %ldms.", (const char *)GetNameNoTime().c_str(), (long)frameMS, (long)sw.Time());
    }

    //logger_base.debug("    Step %s frame %ld done in %ld.", (const char *)GetNameNoTime().c_str(), (long)frameMS, (long)sw.Time());

    if (timesource != nullptr)
    {
        return timesource->Done();
    }
    else
    {
        return frameMS >= GetLengthMS() - GetFrameMS();
    }
}

size_t PlayListStep::GetFrameMS()
{
    size_t ms = 0;
    GetTimeSource(ms);
    return ms;
}

void PlayListStep::Start(int loops)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("         ######## Playlist step %s starting.", (const char*)GetNameNoTime().c_str());

    _loops = loops;
    _startTime = wxGetUTCTimeMillis().GetLo();
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it: _items)
        {
            it->Start(GetLengthMS());
        }
    }
}

void PlayListStep::Pause(bool pause)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (pause)
    {
        _pause = wxGetUTCTimeMillis().GetLo();
        logger_base.info("                  Playlist step %s pausing.", (const char*)GetNameNoTime().c_str());
    }
    else
    {
        logger_base.info("                  Playlist step %s unpausing.", (const char*)GetNameNoTime().c_str());
        _pause = 0;
    }

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it : _items)
        {
            it->Pause(pause);
        }
    }
}

void PlayListStep::Advance(int seconds)
{
    size_t msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);

    if (timesource != nullptr)
    {
        if (timesource->Advance(seconds)) return;
    }

    _startTime -= seconds * 1000;
    if (_startTime > wxGetUTCTimeMillis().GetLo()) _startTime = wxGetUTCTimeMillis().GetLo();
    if (wxGetUTCTimeMillis().GetLo() - _startTime > GetLengthMS()) _startTime = wxGetUTCTimeMillis().GetLo() - GetLengthMS();
}

void PlayListStep::Suspend(bool suspend)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!IsPaused())
    {
        if (suspend)
        {
            _suspend = wxGetUTCTimeMillis().GetLo();
            logger_base.info("                  Playlist step %s suspending.", (const char*)GetNameNoTime().c_str());
        }
        else
        {
            logger_base.info("                  Playlist step %s unsuspending.", (const char*)GetNameNoTime().c_str());
            _suspend = 0;
        }
    }

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it : _items)
        {
            it->Suspend(suspend);
        }
    }
}

void PlayListStep::Restart()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playlist step %s restarting.", (const char*)GetNameNoTime().c_str());

    _startTime = wxGetUTCTimeMillis().GetLo();
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it : _items)
        {
            it->Restart();
        }
    }
}

void PlayListStep::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("         ######## Playlist step %s stopping.", (const char*)GetNameNoTime().c_str());

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it : _items)
        {
            it->Stop();
        }
    }
}

size_t PlayListStep::GetPosition()
{
    size_t msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);

    size_t frameMS;
    if (timesource != nullptr)
    {
        frameMS = timesource->GetPositionMS();
    }
    else
    {
        auto now = wxGetUTCTimeMillis().GetLo();
        if (_pause == 0)
        {
            if (_suspend == 0)
            {
                frameMS = now - _startTime;
            }
            else
            {
                frameMS = now - _startTime - (now - _suspend);
            }
        }
        else
        {
            frameMS = now - _startTime - (now - _pause);
        }
    }

    wxASSERT(frameMS >= 0);
    wxASSERT(frameMS < 100000000);

    return frameMS;
}

size_t PlayListStep::GetLengthMS()
{
    size_t msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);
    if (timesource != nullptr)
    {
        wxASSERT(timesource->GetDurationMS() != 0);
        return timesource->GetDurationMS();
    }
    else
    {
        {
            ReentrancyCounter rec(_reentrancyCounter);
            size_t len = 0;
            for (auto it : _items)
            {
                // duration has to look valid
                if (it->GetDurationMS() < 999999999)
                {
                    len = std::max(len, it->GetDurationMS());
                }
            }

            if (len == 0)
            {
                for (auto it : _items)
                {
                    len = std::max(len, it->GetDurationMS(msPerFrame));
                }

                if (len == 0) len = msPerFrame;
            }
            wxASSERT(len != 0);
            return len;
        }
    }
}

std::string PlayListStep::GetActiveSyncItemFSEQ()
{
    std::string res;
    size_t ms;
    auto ts = GetTimeSource(ms);

    if (ts != nullptr)
    {
        res = ts->GetSyncItemFSEQ();
    }

    if (res == "")
    {
        {
            ReentrancyCounter rec(_reentrancyCounter);
            for (auto it = _items.rbegin(); it != _items.rend(); ++it)
            {
                res = (*it)->GetSyncItemFSEQ();
                if (res != "") break;
            }
        }
    }

    return res;
}

std::string PlayListStep::GetActiveSyncItemMedia()
{
    std::string res;
    size_t ms;
    auto ts = GetTimeSource(ms);

    if (ts != nullptr)
    {
        res = ts->GetSyncItemMedia();
    }

    if (res == "")
    {
        {
            ReentrancyCounter rec(_reentrancyCounter);
            for (auto it = _items.rbegin(); it != _items.rend(); ++it)
            {
                res = (*it)->GetSyncItemMedia();
                if (res != "") break;
            }
        }
    }

    return res;
}

bool PlayListStep::IsRunningFSEQ(const std::string& fseqFile)
{
    std::string fseq = GetActiveSyncItemFSEQ();
    wxFileName fn(fseq);

    return (fn.GetFullName().Lower() == wxString(fseqFile).Lower());
}

void PlayListStep::SetSyncPosition(size_t ms, size_t acceptableJitter, bool force)
{
    static log4cpp::Category &logger_sync = log4cpp::Category::getInstance(std::string("log_sync"));
    logger_sync.debug("SetSyncPosition: MS %ld Force %s.", (long)ms, force ? "true" : "false");

    std::string fseq = GetActiveSyncItemFSEQ();

    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            if ((*it)->GetTitle() == "FSEQ")
            {
                PlayListItemFSEQ* pli = (PlayListItemFSEQ*)(*it);
                if (fseq == pli->GetFSEQFileName())
                {
                    // wxASSERT(abs((long)frame * (long)pli->GetFrameMS() - (long)ms) < pli->GetFrameMS());
                    long posDiff = pli->GetPositionMS() - ms; // Stephen Morgan - Wrong Variable?

                    // double the acceptable jitter if we are ahead as this is more likely a network effect
                    // stops a jump back then big jump forward as often
                    if (posDiff < 0) acceptableJitter *= 2;

                    if (std::abs(posDiff) > acceptableJitter)
                    {
                        int frame = ms / pli->GetFrameMS();
                        if (force)
                        {
                            long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();

                            if (timeDiff != 0) // if this is zero then we are less than one frame out
                            {
                                logger_sync.debug("Sync: Position was %d:%d - should be %d:%d: %ld:%ld. FORCED ReSync.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff, posDiff);

                                if (posDiff > (acceptableJitter * 2)) {
                                    pli->SetPosition(frame, ms);
                                    logger_sync.debug("Way OFF!! Need to SKIP (%ld).", timeDiff); // Add time to current position
                                }
                                else {
                                    long mscorrection = 0;
                                    int  fcorrection = 0;
                                    if (timeDiff > 0) {		// Ahead or Behind? 
                                        mscorrection = (long)(acceptableJitter / 20);
                                        fcorrection = 1;
                                        logger_sync.debug("Behind:(%ld)- Need to move Forward - Correction(%ld).", posDiff, mscorrection); // Add time to current position
                                    }
                                    else {
                                        mscorrection = -(long)(acceptableJitter / 20);
                                        fcorrection = -1;
                                        logger_sync.debug("Ahead:(%ld)- Need to move Back - Correction(%ld).", posDiff, mscorrection); // Subtract time from current position
                                    }

                                    //pli->SetPosition(frame, pli->GetPositionMS() + mscorrection);
                                    pli->SetPosition(pli->GetCurrentFrame() + fcorrection, pli->GetPositionMS() + mscorrection);
                                }

                                if (xScheduleFrame::GetScheduleManager() != nullptr)
                                    xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(0);
                            }
                        }
                        else
                        {
                            long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();

                            // only adjust position if we are more that one frame out of sync
                            if (timeDiff != 0)
                            {
                                int adjustment = 0;
                                if (abs(timeDiff) > pli->GetFrameMS() * 2)
                                {
                                    adjustment = timeDiff / abs(timeDiff) * (int)((float)pli->GetFrameMS() * 0.1);
                                }
                                else if (abs(timeDiff) > pli->GetFrameMS())
                                {
                                    adjustment = timeDiff / abs(timeDiff) * (int)((float)pli->GetFrameMS() * 0.06);
                                }

                                logger_sync.debug("Sync: Position was %d:%d - should be %d:%d: %ld -> Adjustment to frame time %d.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff, adjustment);
                                if (xScheduleFrame::GetScheduleManager() != nullptr)
                                    xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(adjustment);
                            }
                        }
                    }
                    break;
                }
            }
            else if ((*it)->GetTitle() == "FSEQ & Video")
            {
                PlayListItemFSEQVideo* pli = (PlayListItemFSEQVideo*)(*it);
                if (fseq == pli->GetFSEQFileName())
                {
                    //wxASSERT(abs((long)frame * (long)pli->GetFrameMS() - (long)ms) < pli->GetFrameMS());

                    long posDiff = pli->GetPositionMS() - ms; // Stephen Morgan - Wrong Variable?

                    // double the acceptable jitter if we are ahead as this is more likely a network effect
                    // stops a jump back then big jump forward as often
                    if (posDiff < 0) acceptableJitter *= 2;

                    if (std::abs(posDiff) > acceptableJitter)
                    {
                        int frame = ms / pli->GetFrameMS();
                        if (force)
                        {
                            long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();
                            logger_sync.debug("Sync: Position was %d:%d - should be %d:%d: %ld. FORCED.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff);
                            pli->SetPosition(frame, ms);
                            if (xScheduleFrame::GetScheduleManager() != nullptr)
                                xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(0);
                        }
                        else
                        {
                            long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();
                            // only adjust position if we are more that one frame out of sync

                            int adjustment = 0;
                            if (timeDiff == 0)
                            {
                            }
                            else if (abs(timeDiff) > pli->GetFrameMS() * 2)
                            {
                                adjustment = timeDiff / abs(timeDiff) * (int)((float)pli->GetFrameMS() * 0.1);
                            }
                            else if (abs(timeDiff) > pli->GetFrameMS())
                            {
                                adjustment = timeDiff / abs(timeDiff) * (int)((float)pli->GetFrameMS() * 0.06);
                            }

                            logger_sync.debug("Sync: Position was %d:%d - should be %d:%d: %ld -> Adjustment to frame time %d.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff, adjustment);

                            if (xScheduleFrame::GetScheduleManager() != nullptr)
                                xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(adjustment);
                        }
                    }
                    break;
                }
            }
        }
    }
}

void PlayListStep::AdjustTime(wxTimeSpan by)
{
    _startTime += by.GetValue().GetLo();
}

std::string PlayListStep::FormatTime(size_t timems, bool ms) const
{
    if (ms)
    {
        return wxString::Format(wxT("%i:%02i.%03i"), (int)(timems / 60000), (int)((timems % 60000) / 1000), (int)(timems % 1000)).ToStdString();
    }
    else
    {
        return wxString::Format(wxT("%i:%02i"), (int)(timems / 60000), (int)((timems % 60000) / 1000)).ToStdString();
    }
}

std::string PlayListStep::GetStatus(bool ms)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string fps = "Unknown";

    if (GetFrameMS() > 0)
    {
        fps = wxString::Format(wxT("%i"), (int)(1000 / GetFrameMS())).ToStdString();
    }
    
    std::string res = "Time: " + FormatTime(GetPosition(), ms) + " Left: " + FormatTime(GetLengthMS() - GetPosition(), ms) + " FPS: " + fps;

    //logger_base.debug(res);

    return res;
}

std::list<PlayListItem*> PlayListStep::GetItems()
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        _items.sort(compare_priority);
        return _items;
    }
}

bool PlayListStep::IsSimple()
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        if (_items.size() == 0) return true;
        if (_items.size() != 1) return false;

        auto type = _items.front()->GetTitle();

        if (type == "FSEQ" || type == "Audio" || type == "FSEQ & Video") {
            return true;
        }
    }

    return false;
}

PlayListItemText* PlayListStep::GetTextItem(const std::string& name)
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (const auto& it : _items) {
            if (it->GetTitle() == "Text" && wxString(it->GetNameNoTime()).Lower() == wxString(name).Lower()) {
                return (PlayListItemText*)it;
            }
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::GetItem(const std::string item)
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (const auto& it : _items) {
            if (wxString(it->GetNameNoTime()).Lower() == wxString(item).Lower()) {
                return it;
            }
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::GetItem(const wxUint32 id)
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (const auto& it : _items) {
            if (it->GetId() == id) {
                return it;
            }
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::FindRunProcessNamed(const std::string& item)
{
    {
        ReentrancyCounter rec(_reentrancyCounter);
        for (const auto& it : _items)
        {
            if (it->GetTitle() == "Run Process" && wxString(it->GetNameNoTime()).Lower() == wxString(item).Lower())
            {
                return it;
            }
        }
    }

    return nullptr;
}

AudioManager* PlayListStep::GetAudioManager(PlayListItem* pli) const
{
    auto title = pli->GetTitle();
    if (title == "Audio")
    {
        PlayListItemAudio* p = (PlayListItemAudio*)pli;
        return p->GetAudioManager();
    }
    else if (title == "FSEQ")
    {
        PlayListItemFSEQ* p = (PlayListItemFSEQ*)pli;
        return p->GetAudioManager();
    }
    else if (title == "FSEQ & Video")
    {
        PlayListItemFSEQVideo* p = (PlayListItemFSEQVideo*)pli;
        return p->GetAudioManager();
    }

    return nullptr;
}

AudioManager* PlayListStep::GetAudioManager()
{
    size_t ms;
    auto pli = GetTimeSource(ms);

    if (pli != nullptr)
    {
        return GetAudioManager(pli);
    }
    else
    {
        {
            ReentrancyCounter rec(_reentrancyCounter);
            for (auto it = _items.begin(); it != _items.end(); ++it)
            {
                if (GetAudioManager(*it) != nullptr)
                {
                    return GetAudioManager(*it);
                }
            }
        }
    }

    return nullptr;
}

int PlayListStep::GetStepIdFromName(const std::string& step)
{
    if (StartsWith(step, "**id="))
    {
        return wxAtoi(step.substr(5, step.size() - 7));
    }
    return -1;
}

std::string PlayListStep::GetStepNameWithId(int id)
{
    return wxString::Format("**id=%d**", id).ToStdString();
}