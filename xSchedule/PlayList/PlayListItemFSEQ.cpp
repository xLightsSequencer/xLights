#include "PlayListItemFSEQ.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemFSEQPanel.h"

PlayListItemFSEQ::PlayListItemFSEQ(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemFSEQ::Load(node);
}

void PlayListItemFSEQ::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _fseqFile = node->GetAttribute("FSEQFile", "");
    _audioFile = node->GetAttribute("AudioFile", "");
    _overrideAudio = (_audioFile != "");
}

PlayListItemFSEQ::PlayListItemFSEQ() : PlayListItem()
{
    _fseqFile = "";
    _overrideAudio = false;
    _audioFile = "";
}

wxXmlNode* PlayListItemFSEQ::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFSEQ");

    node->AddAttribute("FSEQFile", _fseqFile);

    if (_overrideAudio)
    {
        node->AddAttribute("AudioFile", _audioFile);
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemFSEQ::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFSEQPanel(notebook, this), "FSEQ", true);
}

std::string PlayListItemFSEQ::GetName() const
{
    wxFileName fn(_fseqFile);
    if (fn.GetName() == "")
    {
        return "Video";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}
