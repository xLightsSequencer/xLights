#include "PlayListItem.h"
#include <wx/xml/xml.h>

PlayListItem::PlayListItem(wxXmlNode* node)
{
    _dirty = false;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
    PlayListItem::Load(node);
}

void PlayListItem::Load(wxXmlNode* node)
{
    _delay = wxAtoi(node->GetAttribute("Delay", "0"));
}

PlayListItem::PlayListItem()
{
    _dirty = false;
    _frames = 0;
    _msPerFrame = 50;
    _delay = 0;
}

void PlayListItem::Save(wxXmlNode* node)
{
    node->AddAttribute("Delay", wxString::Format(wxT("%i"), _delay));
}

std::string PlayListItem::GetName() const
{
    std::string duration = "";
    if (GetDurationMS() != 0)
    {
        duration = " [" + wxString::Format(wxT("%.3f"), (float)GetDurationMS() / 1000.0).ToStdString() + "]";
    }
    if (_name != "") return _name + duration;

    return "<unnamed>" + duration;
}

void PlayListItem::Copy(PlayListItem* to) const
{
    to->_dirty = false;
    to->_delay = _delay;
    to->_frames = _frames;
    to->_msPerFrame = _msPerFrame;
    to->_name = _name;
    to->_priority = _priority;
}
