#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/grid.h"

class EzGrid : public wxGrid
{
public:
    EzGrid( wxWindow *parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxWANTS_CHARS,
        const wxString& name = wxPanelNameStr ) :
            wxGrid(parent, id, pos, size, style, name ) {}
private:
    DECLARE_EVENT_TABLE()
    void OnCellLeftClick( wxGridEvent& );
};
