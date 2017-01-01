#include "PlayListItem.h"
#include <wx/xml/xml.h>

PlayListItem::PlayListItem(wxXmlNode* node)
{
    _dirty = false;
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