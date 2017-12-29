#include "PlayListStep.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "PlayListItemVideo.h"
#include "PlayListItemImage.h"
#include "PlayListItemESEQ.h"
#include "PlayListItemFile.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemText.h"
#include "PlayListItemFSEQVideo.h"
#include "PlayListItemTest.h"
#include "PlayListItemRDS.h"
#include "PlayListItemPJLink.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemDelay.h"
#include "PlayListItemRunProcess.h"
#include "PlayListItemCURL.h"
#include "PlayListItemRunCommand.h"
#include "PlayListItemOSC.h"
#include "PlayListItemAudio.h"
#include <wx/filename.h>
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

int __playliststepid = 0;

PlayListStep::PlayListStep(OutputManager* outputManager, wxXmlNode* node)
{
    _loops = -1;
    _pause = 0;
    _suspend = 0;
    _id = __playliststepid++;
    _startTime = 0;
    _framecount = 0;
    _excludeFromRandom = false;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    Load(outputManager, node);
}

bool compare_priority(const PlayListItem* first, const PlayListItem* second)
{
    return first->GetPriority() > second->GetPriority();
}

PlayListStep::PlayListStep()
{
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
}

PlayListStep::PlayListStep(const PlayListStep& step)
{
    _startTime = 0;
    _pause = 0;
    _suspend = 0;
    _loops = step._loops;
    _framecount = step._framecount;
    _name = step._name;
    _lastSavedChangeCount = step._lastSavedChangeCount;
    _changeCount = step._changeCount;
    _excludeFromRandom = step._excludeFromRandom;
    _id = step._id;
    for (auto it = step._items.begin(); it != step._items.end(); ++it)
    {
        _items.push_back((*it)->Copy());
    }
    _items.sort(compare_priority);
}

void PlayListStep::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}

PlayListStep::~PlayListStep()
{
    while (_items.size() > 0)
    {
        auto toremove = _items.front();
        _items.remove(toremove);
        delete toremove;
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

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    return res;
}

void PlayListStep::Load(OutputManager* outputManager, wxXmlNode* node)
{
    _name = node->GetAttribute("Name", "");
    _excludeFromRandom = node->GetAttribute("ExcludeRandom", "FALSE") == "TRUE";

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
        else if (n->GetName() == "PLIAudio")
        {
            _items.push_back(new PlayListItemAudio(n));
        }
        else if (n->GetName() == "PLIFSEQVideo")
        {
            _items.push_back(new PlayListItemFSEQVideo(outputManager, n));
        }
        else if (n->GetName() == "PLITest")
        {
            _items.push_back(new PlayListItemTest(outputManager, n));
        }
        else if (n->GetName() == "PLIRDS")
        {
            _items.push_back(new PlayListItemRDS(n));
        }
        else if (n->GetName() == "PLIPJLink")
        {
            _items.push_back(new PlayListItemPJLink(n));
        }
        else if (n->GetName() == "PLIESEQ")
        {
            _items.push_back(new PlayListItemESEQ(n));
        }
        else if (n->GetName() == "PLIImage")
        {
            _items.push_back(new PlayListItemImage(n));
        }
        else if (n->GetName() == "PLIAllSet")
        {
            _items.push_back(new PlayListItemAllOff(outputManager, n));
        }
        else if (n->GetName() == "PLIDelay")
        {
            _items.push_back(new PlayListItemDelay(n));
        }
        else if (n->GetName() == "PLIProcess")
        {
            _items.push_back(new PlayListItemRunProcess(n));
        }
        else if (n->GetName() == "PLICURL")
        {
            _items.push_back(new PlayListItemCURL(n));
        }
        else if (n->GetName() == "PLICommand")
        {
            _items.push_back(new PlayListItemRunCommand(n));
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
}

bool PlayListStep::IsDirty() const
{
    bool res = _lastSavedChangeCount != _changeCount;

    auto it = _items.begin();
    while (!res && it != _items.end())
    {
        res = res || (*it)->IsDirty();
        ++it;
    }

    return res;
}

void PlayListStep::RemoveItem(PlayListItem* item)
{
    _items.remove(item);
    _changeCount++;
}

std::string PlayListStep::GetName() const
{
    std::string duration = "";
    if (GetLengthMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetLengthMS() / 1000.0).ToStdString() + "]";
    }

    if (_name != "") return _name + duration;

    if (_items.size() == 0) return "<unnamed>" + duration;

    return _items.front()->GetNameNoTime() + duration;
}

std::string PlayListStep::GetNameNoTime() const
{
    if (_name != "") return _name;

    if (_items.size() == 0) return "<unnamed>";

    return _items.front()->GetNameNoTime();
}

PlayListItem* PlayListStep::GetTimeSource(size_t &ms) const
{
    ms = 9999;
    PlayListItem* timesource = nullptr;

    // Prioritise the highest priotity FSEQ for timing
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->GetTitle() == "FSEQ" || (*it)->GetTitle() == "FSEQ & Video")
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
        else if (timesource == nullptr)
        {
            if (msec != 0 && msec < ms)
            {
                ms = msec;
            }
        }
    }

    if (ms == 9999)
    {
        ms = 50;
    }

    return timesource;
}

bool PlayListStep::Frame(wxByte* buffer, size_t size, bool outputframe)
{
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

size_t PlayListStep::GetFrameMS() const
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
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Start(GetLengthMS());
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

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Pause(pause);
    }
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

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Suspend(suspend);
    }
}

void PlayListStep::Restart()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Playlist step %s restarting.", (const char*)GetNameNoTime().c_str());

    _startTime = wxGetUTCTimeMillis().GetLo();
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Restart();
    }
}

void PlayListStep::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("         ######## Playlist step %s stopping.", (const char*)GetNameNoTime().c_str());

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Stop();
    }
}

size_t PlayListStep::GetPosition() const
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

    return frameMS;
}

size_t PlayListStep::GetLengthMS() const
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
        size_t len = 0;
        for (auto it = _items.begin(); it != _items.end(); ++it)
        {
            len = std::max(len, (*it)->GetDurationMS());
        }

        if (len == 0)
        {
            for (auto it = _items.begin(); it != _items.end(); ++it)
            {
                len = std::max(len, (*it)->GetDurationMS(msPerFrame));
            }

            if (len == 0) len = msPerFrame;
        }

        wxASSERT(len != 0);
        return len;
    }
}

std::string PlayListStep::GetActiveSyncItemFSEQ() const
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
        for (auto it = _items.rbegin(); it != _items.rend(); ++it)
        {
            res = (*it)->GetSyncItemFSEQ();
            if (res != "") break;
        }
    }

    return res;
}

std::string PlayListStep::GetActiveSyncItemMedia() const
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
        for (auto it = _items.rbegin(); it != _items.rend(); ++it)
        {
            res = (*it)->GetSyncItemMedia();
            if (res != "") break;
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

// Because frame and ms dont seem to be properly correlated #defining USEMS will derive the frame from the time.
// not defining it will ignore the ms and rely on the frame
// #define USEMS

void PlayListStep::SetSyncPosition(size_t frame, size_t ms, bool force)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SetSyncPosition: Frame %ld MS %ld Force %s.", (long)frame, (long)ms, force? "true" : "false");

    std::string fseq = GetActiveSyncItemFSEQ();

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->GetTitle() == "FSEQ")
        {
            PlayListItemFSEQ* pli = (PlayListItemFSEQ*)(*it);
            if (fseq == pli->GetFSEQFileName())
            {
                // wxASSERT(abs((long)frame * (long)pli->GetFrameMS() - (long)ms) < pli->GetFrameMS());

#ifdef USEMS
                frame = ms / pli->GetFrameMS();
                logger_base.debug("    Adjusted: Frame %ld.", (long)frame);
#endif

                if (force)
                {
                    long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();
                    logger_base.debug("Sync: Position was %d:%d - should be %d:%d: %ld. FORCED.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff);
                    pli->SetPosition(frame, ms);
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

                    logger_base.debug("Sync: Position was %d:%d - should be %d:%d: %ld -> Adjustment to frame time %d.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff, adjustment);
                    xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(adjustment);
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
#ifdef USEMS
                frame = ms / pli->GetFrameMS();
                logger_base.debug("    Adjusted: Frame %ld.", (long)frame);
#endif

                if (force)
                {
                    long timeDiff = (long)frame * (long)pli->GetFrameMS() - (long)pli->GetPositionMS();
                    logger_base.debug("Sync: Position was %d:%d - should be %d:%d: %ld. FORCED.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff);
                    pli->SetPosition(frame, ms);
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

                    logger_base.debug("Sync: Position was %d:%d - should be %d:%d: %ld -> Adjustment to frame time %d.", pli->GetCurrentFrame(), pli->GetPositionMS(), frame, frame * pli->GetFrameMS(), timeDiff, adjustment);
                    xScheduleFrame::GetScheduleManager()->SetTimerAdjustment(adjustment);
                }
                break;
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

std::string PlayListStep::GetStatus(bool ms) const
{
    std::string fps = "Unknown";

    if (GetFrameMS() > 0)
    {
        fps = wxString::Format(wxT("%i"), (int)(1000 / GetFrameMS())).ToStdString();
    }
    
    return "Time: " + FormatTime(GetPosition(), ms) + " Left: " + FormatTime(GetLengthMS() - GetPosition(), ms) + " FPS: " + fps;
}

std::list<PlayListItem*> PlayListStep::GetItems()
{
    _items.sort(compare_priority);
    return _items;
}

bool PlayListStep::IsSimple() const
{
    if (_items.size() != 1) return false;

    auto type = _items.front()->GetTitle();

    if (type == "FSEQ" || type == "Audio" || type == "FSEQ & Video")
    {
        return true;
    }

    return false;
}

PlayListItemText* PlayListStep::GetTextItem(const std::string& name) const
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->GetTitle() == "Text" && wxString((*it)->GetNameNoTime()).Lower() == wxString(name).Lower())
        {
            return (PlayListItemText*)*it;
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::GetItem(const std::string item) const
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if (wxString((*it)->GetNameNoTime()).Lower() == wxString(item).Lower())
        {
            return *it;
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::GetItem(const wxUint32 id) const
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return *it;
        }
    }

    return nullptr;
}

PlayListItem* PlayListStep::FindRunProcessNamed(const std::string& item) const
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->GetTitle() == "Run Process" && wxString((*it)->GetNameNoTime()).Lower() == wxString(item).Lower())
        {
            return (*it);
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

AudioManager* PlayListStep::GetAudioManager() const
{
    size_t ms;
    auto pli = GetTimeSource(ms);

    if (pli != nullptr)
    {
        return GetAudioManager(pli);
    }
    else
    {
        for (auto it=  _items.begin(); it != _items.end(); ++it)
        {
            if (GetAudioManager(*it) != nullptr)
            {
                return GetAudioManager(*it);
            }
        }
    }

    return nullptr;
}



