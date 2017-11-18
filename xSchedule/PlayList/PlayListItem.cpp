#include "PlayListItem.h"
#include <wx/xml/xml.h>
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

int __playlistitemid = 0;

PlayListItem::PlayListItem(wxXmlNode* node)
{
    _currentFrame = 0;
    _id = __playlistitemid++;
    _priority = 0;
    _volume = -1;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    _name = "";
    PlayListItem::Load(node);
}

void PlayListItem::Load(wxXmlNode* node)
{
    _delay = wxAtoi(node->GetAttribute("Delay", "0"));
    _volume = wxAtoi(node->GetAttribute("Volume", "-1"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
    _name = node->GetAttribute("Name", "");
}

PlayListItem::PlayListItem()
{
    _currentFrame = 0;
    _name = "";
    _id = __playlistitemid++;
    _volume = -1;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    _priority = 0;
}

void PlayListItem::Save(wxXmlNode* node)
{
    node->AddAttribute("Delay", wxString::Format(wxT("%i"), (long)_delay));
    node->AddAttribute("Name", _name);
    if (_volume != -1)
    {
        node->AddAttribute("Volume", wxString::Format(wxT("%i"), _volume));
    }
    if (_priority != 0)
    {
        node->AddAttribute("Priority", wxString::Format(wxT("%i"), (int)_priority));
    }
}

std::string PlayListItem::GetName() const
{
    std::string duration = "";
    if (GetDurationMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetDurationMS() / 1000.0).ToStdString() + "]";
    }

    return GetNameNoTime() + duration;
}

std::string PlayListItem::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "<unnamed>";
}

void PlayListItem::Copy(PlayListItem* to) const
{
    to->_id = _id;
    to->_lastSavedChangeCount = _lastSavedChangeCount;
    to->_changeCount = _changeCount;
    to->_delay = _delay;
    to->_frames = _frames;
    to->_msPerFrame = _msPerFrame;
    to->_name = _name;
    to->_priority = _priority;
    to->_volume = _volume;
}

bool PlayListItem::IsInSlaveMode() const
{
    ScheduleManager* sm = xScheduleFrame::GetScheduleManager();

    if (sm == nullptr) return false;

    SYNCMODE m = sm->GetMode();

    return (m == SYNCMODE::FPPSLAVE || m == SYNCMODE::ARTNETSLAVE || m == SYNCMODE::FPPUNICASTSLAVE);
}
