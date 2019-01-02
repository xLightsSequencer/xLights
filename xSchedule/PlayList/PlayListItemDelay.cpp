#include "PlayListItemDelay.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemDelayPanel.h"

PlayListItemDelay::PlayListItemDelay(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemDelay::Load(node);
}

void PlayListItemDelay::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtol(node->GetAttribute("Duration", "50"));
}

PlayListItemDelay::PlayListItemDelay() : PlayListItem()
{
    _duration = 50;
    SetName("Delay");
}

PlayListItem* PlayListItemDelay::Copy() const
{
    PlayListItemDelay* res = new PlayListItemDelay();
    res->_duration = _duration;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemDelay::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIDelay");

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
   
    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemDelay::GetTitle() const
{
    return "Delay";
}

void PlayListItemDelay::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemDelayPanel(notebook, this), GetTitle(), true);
}

void PlayListItemDelay::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    
}
