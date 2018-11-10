#include "NullOutput.h"

#include <wx/xml/xml.h>
#ifndef EXCLUDENETWORKUI
#include "NullOutputDialog.h"
#endif
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
    res += "(" + std::string(wxString::Format(wxT("%d"), _startChannel)) + "-" + std::string(wxString::Format(wxT("%d"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string NullOutput::GetChannelMapping(int32_t ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%d"), ch)) + " maps to ...\n";
    res += "Type: NULL (" + std::string(wxString::Format(wxT("%d"), _nullNumber)) + ")\nChannel: " + std::string(wxString::Format(wxT("%d"), ch - _startChannel)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}

std::string NullOutput::GetSortName() const
{
    return wxString::Format("NULL%02d", _nullNumber).ToStdString();
}

std::string NullOutput::GetExport() const
{
    // "Output Number,Start Absolute,End Absolute,Type,IP,Multicast,Universe/Id,Comm Port,Baud Rate,Description,Channels,Active,Suppress Duplicates,Auto Size,
    // FPP Proxy,Keep Channel Numbers,Channels Per Packet,Port,Dont Configure,Priority,Vendor,Model,Supports Virtual Strings,Supports Smart Remotes";
    return wxString::Format("%d,%ld,%ld,%s,,,%d,,,%s,%ld,,,,,,,,,,,,,",
        _outputNumber, GetStartChannel(), GetEndChannel(), GetType(), GetId(), _description, _channels).ToStdString();
}

#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
Output* NullOutput::Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager)
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
