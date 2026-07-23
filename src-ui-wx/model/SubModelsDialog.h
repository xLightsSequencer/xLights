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
#include <wx/dnd.h>
#include <wx/listctrl.h>

// EVT_SMDROP and SubModelTextDropTarget are defined here (not in SubModelsPanel.h)
// because SubModelsPanel.cpp #includes this header to get the class declaration,
// and the event + target implementations live in SubModelsDialog.cpp.
wxDECLARE_EVENT(EVT_SMDROP, wxCommandEvent);

class SubModelTextDropTarget : public wxTextDropTarget
{
public:
    SubModelTextDropTarget(wxWindow* owner, wxListCtrl* list, wxString type)
    {
        _owner = owner;
        _list  = list;
        _type  = type;
    }

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow*   _owner;
    wxListCtrl* _list;
    wxString    _type;
};

class Model;
class ModelPreview;
class SubModelsPanel;
class OutputManager;

class SubModelsDialog : public wxDialog
{
public:
    SubModelsDialog(wxWindow* parent, OutputManager* outputManager);
    virtual ~SubModelsDialog();

    void Setup(Model* m);
    void Save();

    bool ReloadLayout = false;

private:
    void OnOK(wxCommandEvent&);
    void OnCancelButton(wxCommandEvent&);
    void OnClose(wxCloseEvent&);
    void ConfirmClose();

    SubModelsPanel* _subModelsPanel = nullptr;
    ModelPreview*   _modelPreview   = nullptr;
    OutputManager*  _outputManager  = nullptr;
    bool            _oldOutputToLights = false;

    DECLARE_EVENT_TABLE()
};
