#include "PlayListItemAllOff.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemAllOffPanel.h"

PlayListItemAllOff::PlayListItemAllOff(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemAllOff::Load(node);
}

void PlayListItemAllOff::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _value = wxAtoi(node->GetAttribute("Value", "0"));
}

PlayListItemAllOff::PlayListItemAllOff() : PlayListItem()
{
    _duration = 50;
    SetName("All Set");
}

PlayListItem* PlayListItemAllOff::Copy() const
{
    PlayListItemAllOff* res = new PlayListItemAllOff();
    res->_duration = _duration;
    res->_blendMode = _blendMode;
    res->_value = _value;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemAllOff::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIAllSet");

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), _duration));
    node->AddAttribute("value", wxString::Format(wxT("%i"), _value));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemAllOff::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAllOffPanel(notebook, this), "All Set", true);
}

void PlayListItemAllOff::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems)
{
    if (ms >= _delay && ms <= _delay + _duration)
    {
        memset(buffer, _value, size);
    }
}
