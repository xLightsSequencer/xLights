#include "PlayListItemRunProcess.h"
#include "PlayListItemRunProcessPanel.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>

PlayListItemRunProcess::PlayListItemRunProcess(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemRunProcess::Load(node);
}

void PlayListItemRunProcess::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _waitForCompletion = (node->GetAttribute("Wait", "FALSE") == "TRUE");
}

PlayListItemRunProcess::PlayListItemRunProcess() : PlayListItem()
{
    _command = "";
    _waitForCompletion = false;
}

wxXmlNode* PlayListItemRunProcess::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIProcess");

    node->AddAttribute("Command", _command);
    if (_waitForCompletion)
    {
        node->AddAttribute("Wait", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemRunProcess::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemRunProcessPanel(notebook, this), "Run Process", true);
}