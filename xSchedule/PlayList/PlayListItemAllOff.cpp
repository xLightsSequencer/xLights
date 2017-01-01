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
}

PlayListItemAllOff::PlayListItemAllOff() : PlayListItem()
{
    _duration = 50;
    SetName("All Off");
}

wxXmlNode* PlayListItemAllOff::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIAllOff");

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), _duration));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemAllOff::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAllOffPanel(notebook, this), "All Off", true);
}

