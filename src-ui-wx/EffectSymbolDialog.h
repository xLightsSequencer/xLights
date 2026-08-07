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

#include <wx/dialog.h>
#include <wx/listctrl.h>

class wxButton;
class SequenceElements;
class EffectSymbolManager;

class EffectSymbolDialog : public wxDialog
{
public:
    EffectSymbolDialog(wxWindow* parent, SequenceElements* sequenceElements);
    virtual ~EffectSymbolDialog();

private:
    void PopulateSymbolList();
    void OnDeleteClick(wxCommandEvent& event);
    void OnRenameClick(wxCommandEvent& event);
    void OnCloseClick(wxCommandEvent& event);
    void OnSymbolSelected(wxListEvent& event);
    void OnSymbolDeselected(wxListEvent& event);
    void UpdateButtonStates();

    wxListCtrl* m_symbolList;
    wxButton* m_deleteButton;
    wxButton* m_renameButton;
    wxButton* m_closeButton;

    SequenceElements* m_sequenceElements;

    DECLARE_EVENT_TABLE()
};
