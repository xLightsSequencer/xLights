#include "PlayListItemVideo.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemVideoPanel.h"

PlayListItemVideo::PlayListItemVideo(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemVideo::Load(node);
}

void PlayListItemVideo::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _videoFile = node->GetAttribute("VideoFile", "");
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
}

PlayListItemVideo::PlayListItemVideo() : PlayListItem()
{
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(100);
    _size.SetHeight(100);
}

wxXmlNode* PlayListItemVideo::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIVideo");

    node->AddAttribute("VideoFile", _videoFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemVideo::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemVideoPanel(notebook, this), "Video", true);
}

std::string PlayListItemVideo::GetName() const
{
    wxFileName fn(_videoFile);
    if (fn.GetName() == "")
    {
        return "Video";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}
