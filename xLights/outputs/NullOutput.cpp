#include "NullOutput.h"

#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>
#include "NullOutputDialog.h"

wxXmlNode* NullOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Output::Save(node);

    return node;
}

std::string NullOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL (" + std::string(wxString::Format(wxT("%i"), _nullNumber)) + ") ";
    res += "(" + std::string(wxString::Format(wxT("%i"), _startChannel)) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string NullOutput::GetChannelMapping(int ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";
    res += "Type: NULL (" + std::string(wxString::Format(wxT("%i"), _nullNumber)) + ")\nChannel: " + std::string(wxString::Format(wxT("%i"), ch - _startChannel)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}

#ifndef EXCLUDENETWORKUI
int NullOutput::Configure(wxWindow* parent, OutputManager& outputManager)
{
    NullOutputDialog dlg(parent);

    dlg.NumChannelsSpinCtrl->SetValue(_channels);
    dlg.TextCtrl_Description->SetValue(_description);
    //dlg.Choice_Controller->SetStringValue(_controller->GetId());

    int res = dlg.ShowModal();
    
    if (res == wxID_OK)
    {
        _description = dlg.TextCtrl_Description->GetValue().ToStdString();
        _channels = dlg.NumChannelsSpinCtrl->GetValue();
    }

    return res;
}
#endif


