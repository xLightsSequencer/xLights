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

//(*Headers(FindDataPanel)
#include <wx/panel.h>
class wxFlexGridSizer;
class wxListView;
//*)

#include <wx/listctrl.h>

#include "sequencer/EffectsGrid.h"

class FindDataPanel : public wxPanel
{
    std::vector<findDataEffect> _foundEffects;
    const EffectsGrid* _effectsGrid = nullptr;

public:
    FindDataPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~FindDataPanel();
    void UpdateEffects(const std::vector<findDataEffect>& effects, const EffectsGrid* eg);

    //(*Declarations(FindDataPanel)
    wxListView* ListView_FoundEffects;
    //*)

protected:
    //(*Identifiers(FindDataPanel)
    static const long ID_LISTVIEW1;
    //*)

private:
    //(*Handlers(FindDataPanel)
    void OnListView_FoundEffectsItemActivated(wxListEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};
