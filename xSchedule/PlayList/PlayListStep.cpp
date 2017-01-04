#include "PlayListStep.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "PlayListItemVideo.h"
#include "PlayListItemImage.h"
#include "PlayListItemESEQ.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemDelay.h"
#include "PlayListItemRunProcess.h"

PlayListStep::PlayListStep(wxXmlNode* node)
{
    _framecount = 0;
    _excludeFromRandom = false;
    _dirty = false;
    Load(node);
}

PlayListStep::PlayListStep()
{
    _framecount = 0;
    _name = "";
    _dirty = false;
    _excludeFromRandom = false;
}

PlayListStep::PlayListStep(const PlayListStep& step)
{
    _framecount = step._framecount;
    _name = step._name;
    _dirty = false;
    _excludeFromRandom = step._excludeFromRandom;
    for (auto it = step._items.begin(); it != step._items.end(); ++it)
    {
        _items.push_back((*it)->Copy());
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

void PlayListStep::Load(wxXmlNode* node)
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
            _items.push_back(new PlayListItemFSEQ(n));
        }
        else if (n->GetName() == "PLIESEQ")
        {
            _items.push_back(new PlayListItemESEQ(n));
        }
        else if (n->GetName() == "PLIImage")
        {
            _items.push_back(new PlayListItemImage(n));
        }
        else if (n->GetName() == "PLIAllOff")
        {
            _items.push_back(new PlayListItemAllOff(n));
        }
        else if (n->GetName() == "PLIDelay")
        {
            _items.push_back(new PlayListItemDelay(n));
        }
        else if (n->GetName() == "PLIProcess")
        {
            _items.push_back(new PlayListItemRunProcess(n));
        }
    }
}

bool PlayListStep::IsDirty() const
{
    bool res = _dirty;

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
    _dirty = true;
}

std::string PlayListStep::GetName() const
{
    if (_name != "") return _name;

    if (_items.size() == 0) return "<unnamed>";

    return _items.front()->GetName();
}

PlayListItem* PlayListStep::GetTimeSource(int &ms) const
{
    PlayListItem* timesource = nullptr;
    ms = 1000;
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        if ((*it)->ControlsTiming())
        {
            if (timesource == nullptr)
            {
                timesource = *it;
                ms = (*it)->GetFrameMS();
            }
            else
            {
                if ((*it)->GetPriority() > timesource->GetPriority())
                {
                    timesource = *it;
                    ms = (*it)->GetFrameMS();
                }
            }
        }
        else if (timesource == nullptr)
        {
            if ((*it)->GetFrameMS() < ms)
            {
                ms = (*it)->GetFrameMS();
            }
        }
           
    }

    if (timesource == nullptr)
    {
        timesource = _items.front();
    }

    return timesource;
}

bool PlayListStep::Frame(wxByte* buffer, size_t size)
{
    int msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);

    size_t frameMS;
    if (timesource != nullptr)
    {
        frameMS = timesource->GetPositionMS();
    }
    else
    {
        frameMS = _framecount * msPerFrame;
        _framecount++;
    }

    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Frame(buffer, size, frameMS, msPerFrame);
    }

    return timesource->Done();
}

void PlayListStep::Start()
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Start();
    }
}

void PlayListStep::Stop()
{
    for (auto it = _items.begin(); it != _items.end(); ++it)
    {
        (*it)->Stop();
    }
}

size_t PlayListStep::GetPosition() const
{
    int msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);

    size_t frameMS;
    if (timesource != nullptr)
    {
        frameMS = timesource->GetPositionMS();
    }
    else
    {
        frameMS = _framecount * msPerFrame;
    }

    return frameMS;
}

size_t PlayListStep::GetLengthMS() const
{
    int msPerFrame = 1000;
    PlayListItem* timesource = GetTimeSource(msPerFrame);
    return timesource->GetDurationMS();
}
