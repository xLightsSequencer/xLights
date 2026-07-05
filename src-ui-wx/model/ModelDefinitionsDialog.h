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
#include <wx/notebook.h>
#include <string>
#include <utility>
#include <vector>

class ModelFacesPanel;
class ModelStatesPanel;
class SubModelsPanel;
class ModelPreview;
class OutputManager;
class Model;

enum ModelDefinitionsTab {
    TAB_FACES     = 0,
    TAB_STATES    = 1,
    TAB_SUBMODELS = 2
};

class ModelDefinitionsDialog : public wxDialog
{
public:
    ModelDefinitionsDialog(wxWindow* parent, OutputManager* outputManager,
                           Model* model, int initialTab = TAB_FACES);
    virtual ~ModelDefinitionsDialog();

    bool ReloadLayout = false;
    bool IsDirty() const { return _isDirty; }
    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& GetRenamedFaces() const;

private:
    void OnInit(wxInitDialogEvent& event);
    void OnNotebookPageChanged(wxBookCtrlEvent& event);
    void OnOK(wxCommandEvent&);
    void OnCancelButton(wxCommandEvent&);
    void OnClose(wxCloseEvent&);
    void ConfirmClose();
    void NotifyDirty();
    void CommitAllChanges();

    wxNotebook*      _notebook       = nullptr;
    ModelFacesPanel* _facesPanel     = nullptr;
    ModelStatesPanel* _statesPanel   = nullptr;
    SubModelsPanel*  _subModelsPanel = nullptr;
    ModelPreview*    _modelPreview   = nullptr;

    OutputManager*   _outputManager     = nullptr;
    Model*           _model             = nullptr;
    bool             _isDirty           = false;
    bool             _oldOutputToLights = false;

    DECLARE_EVENT_TABLE()
};
