#include "NullOutput.h"

#include <wx/xml/xml.h>
#include "NullOutputDialog.h"

#pragma region Getters and Setters
std::string NullOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL (" + std::string(wxString::Format(wxT("%i"), _nullNumber)) + ") ";
    res += "(" + std::string(wxString::Format(wxT("%i"), (long)_startChannel)) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string NullOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";
    res += "Type: NULL (" + std::string(wxString::Format(wxT("%i"), _nullNumber)) + ")\nChannel: " + std::string(wxString::Format(wxT("%i"), ch - _startChannel)) + "\n";

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