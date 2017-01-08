#include "PlayListItem.h"
#include <wx/xml/xml.h>

PlayListItem::PlayListItem(wxXmlNode* node)
{
    _priority = 0;
    _volume = -1;
    _dirty = false;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    PlayListItem::Load(node);
}

void PlayListItem::Load(wxXmlNode* node)
{
    _delay = wxAtoi(node->GetAttribute("Delay", "0"));
    _volume = wxAtoi(node->GetAttribute("Volume", "-1"));
    _priority = wxAtoi(node->GetAttribute("Priority", "0"));
}

PlayListItem::PlayListItem()
{
    _volume = -1;
    _dirty = false;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    _priority = 0;
}

void PlayListItem::Save(wxXmlNode* node)
{
    node->AddAttribute("Delay", wxString::Format(wxT("%i"), _delay));
    if (_volume != -1)
    {
        node->AddAttribute("Volume", wxString::Format(wxT("%i"), _volume));
    }
    if (_priority != 0)
    {
        node->AddAttribute("Priority", wxString::Format(wxT("%i"), _priority));
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
    to->_dirty = false;
    to->_delay = _delay;
    to->_frames = _frames;
    to->_msPerFrame = _msPerFrame;
    to->_name = _name;
    to->_priority = _priority;
    to->_volume = _volume;
}
