#include "NullOutput.h"

#include <wx/xml/xml.h>
#include "NullOutputDialog.h"

#pragma region Save
void NullOutput::Save(wxXmlNode* node)
{
    node->AddAttribute("Id", wxString::Format(wxT("%i"), GetId()));

    Output::Save(node);
}

wxXmlNode* NullOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
NullOutput::NullOutput(wxXmlNode* node) : Output(node)
{
    SetId(wxAtoi(node->GetAttribute("Id", "0")));
}

std::string NullOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL (" + std::string(wxString::Format(wxT("%i"), _nullNumber)) + ") ";
    res += "(" + std::string(wxString::Format(wxT("%ld"), (long)_startChannel)) + "-" + std::string(wxString::Format(wxT("%ld"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string NullOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%ld"), ch)) + " maps to ...\n";
    res += "Type: NULL (" + std::string(wxString::Format(wxT("%ld"), _nullNumber)) + ")\nChannel: " + std::string(wxString::Format(wxT("%ld"), ch - _startChannel)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}
#pragma endregion Getters and Setters


#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* NullOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    NullOutputDialog dlg(parent, this, outputManager);

    int res = dlg.ShowModal();
    
    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return this;
}
#endif
#pragma endregion UI
