#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>

class MHColorPanel : public wxPanel
{
public:
    MHColorPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
    :    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS)
    {}
    
    virtual ~MHColorPanel() {};

    virtual std::string GetColour() = 0;
    virtual bool HasColour() = 0;
    virtual void SetColours( const std::string& _colors ) = 0;
    virtual void ResetColours() = 0;

private:

};

