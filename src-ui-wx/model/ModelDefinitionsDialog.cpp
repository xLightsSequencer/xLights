/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelDefinitionsDialog.h"
#include "ModelFacesPanel.h"
#include "ModelStatesPanel.h"
#include "SubModelsPanel.h"
#include "layout/ModelPreview.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/msgdlg.h>
#include "shared/utils/wxUtilities.h"

BEGIN_EVENT_TABLE(ModelDefinitionsDialog, wxDialog)
    EVT_INIT_DIALOG(                ModelDefinitionsDialog::OnInit)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, ModelDefinitionsDialog::OnNotebookPageChanged)
    EVT_BUTTON(wxID_OK,     ModelDefinitionsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ModelDefinitionsDialog::OnCancelButton)
    EVT_CLOSE(              ModelDefinitionsDialog::OnClose)
END_EVENT_TABLE()

ModelDefinitionsDialog::ModelDefinitionsDialog(wxWindow* parent, OutputManager* outputManager,
                                               Model* model, int initialTab)
    : wxDialog(parent, wxID_ANY, "Model Definitions", wxDefaultPosition, wxDefaultSize,
               wxCAPTION | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
      _outputManager(outputManager),
      _model(model)
{
    auto* outerSizer = new wxFlexGridSizer(2, 1, 0, 0);
    outerSizer->AddGrowableCol(0);
    outerSizer->AddGrowableRow(0);

    auto* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxSP_3D | wxSP_LIVE_UPDATE);
    splitter->SetSashGravity(0.6);

    // Left: notebook with three tabs
    _notebook = new wxNotebook(splitter, wxID_ANY);

    _facesPanel = new ModelFacesPanel(_notebook, outputManager);
    _statesPanel = new ModelStatesPanel(_notebook, outputManager);
    _subModelsPanel = new SubModelsPanel(_notebook, outputManager);

    _notebook->AddPage(_facesPanel,     "Faces",     false);
    _notebook->AddPage(_statesPanel,    "States",    false);
    _notebook->AddPage(_subModelsPanel, "SubModels", false);

    // Right: preview panel
    auto* previewPanel = new wxPanel(splitter, wxID_ANY);
    auto* previewSizer = new wxFlexGridSizer(1, 1, 0, 0);
    previewSizer->AddGrowableCol(0);
    previewSizer->AddGrowableRow(0);

    _modelPreview = new ModelPreview(previewPanel);
    _modelPreview->SetMinSize(wxSize(150, 150));
    previewSizer->Add(_modelPreview, 1, wxALL | wxEXPAND, 0);
    previewPanel->SetSizer(previewSizer);

    splitter->SplitVertically(_notebook, previewPanel);
    outerSizer->Add(splitter, 1, wxALL | wxEXPAND, 0);

    auto* btnSizer = new wxStdDialogButtonSizer();
    btnSizer->AddButton(new wxButton(this, wxID_OK));
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
    btnSizer->Realize();

    auto* btnRow = new wxBoxSizer(wxHORIZONTAL);
    btnRow->AddStretchSpacer(1);
    btnRow->Add(btnSizer, 0, wxALL, 5);
    btnRow->AddStretchSpacer(1);
    outerSizer->Add(btnRow, 0, wxALL | wxEXPAND, 0);

    SetSizer(outerSizer);
    outerSizer->Fit(this);
    outerSizer->SetSizeHints(this);
    Center();

    // Wire shared preview and dirty-state callbacks to all panels
    _facesPanel->SetModelPreview(_modelPreview);
    _statesPanel->SetModelPreview(_modelPreview);
    _subModelsPanel->SetModelPreview(_modelPreview);

    _facesPanel->SetChangeCallback([this]{ NotifyDirty(); });
    _statesPanel->SetChangeCallback([this]{ NotifyDirty(); });
    _subModelsPanel->SetChangeCallback([this]{ NotifyDirty(); });

    // Wire cross-tab submodel access so panels read in-flight data without touching _model
    auto getSubModelNames  = [this]{ return _subModelsPanel->GetCurrentSubModelNames(); };
    auto getSubModelRanges = [this](const std::string& n){ return _subModelsPanel->GetSubModelRanges(n); };
    _facesPanel->SetSubModelCallbacks(getSubModelNames, getSubModelRanges);
    _statesPanel->SetSubModelCallbacks(getSubModelNames, getSubModelRanges);

    // Wire cross-tab state access so the Faces state-outline overlay reflects
    // in-flight (not-yet-saved) States tab edits.
    _facesPanel->SetStateInfoCallback([this] { return _statesPanel->GetStateInfo(); });

    // Load data into panels
    _originalFaceInfo = _model->GetFaceInfo();
    _originalStateInfo = _model->GetStateInfo();
    _facesPanel->SetFaceInfo(_model, _originalFaceInfo);
    _statesPanel->SetStateInfo(_model, _originalStateInfo);
    // Seed the state-outline dropdown up front - it's otherwise only refreshed
    // on a tab switch into Faces, leaving it empty when Faces is the initial tab.
    _facesPanel->RefreshStateOutlineChoice(_statesPanel->GetStateInfo());
    _subModelsPanel->Setup(_model);

    // Select initial tab and activate it. ChangeSelection() (unlike SetSelection())
    // does not fire EVT_NOTEBOOK_PAGE_CHANGED, so OnActivate() below is the only
    // trigger - avoiding a double-bind of the preview mouse handlers.
    _notebook->ChangeSelection(initialTab);
    switch (initialTab) {
        case TAB_FACES:     _facesPanel->OnActivate();     break;
        case TAB_STATES:    _statesPanel->OnActivate();    break;
        case TAB_SUBMODELS: _subModelsPanel->OnActivate(); break;
    }

    SetEscapeId(wxID_CANCEL);
    EnableCloseButton(false);

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
        SetConfigBool("OutputActive", false);
    }
}

ModelDefinitionsDialog::~ModelDefinitionsDialog()
{
    SaveWindowPosition("xLightsSubModelDialogPosition", this);
    if (_oldOutputToLights) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
    }
}

void ModelDefinitionsDialog::OnInit(wxInitDialogEvent& event)
{
    SetSize(1200, 800);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsSubModelDialogPosition", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400) sz.SetWidth(400);
        if (sz.GetHeight() < 300) sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
    }
    EnsureWindowHeaderIsOnScreen(this);
    Layout();
}

void ModelDefinitionsDialog::NotifyDirty()
{
    _isDirty = true;
}

const std::vector<std::pair<std::string, std::string>>& ModelDefinitionsDialog::GetRenamedFaces() const
{
    return _facesPanel->GetRenamedFaces();
}

bool ModelDefinitionsDialog::GetReloadLayout() const
{
    return _subModelsPanel->ReloadLayout;
}

void ModelDefinitionsDialog::CommitAllChanges()
{
    bool facesChanged = _facesPanel->GetFaceInfo() != _originalFaceInfo;
    bool statesChanged = _statesPanel->GetStateInfo() != _originalStateInfo;
    bool subModelsChanged = _subModelsPanel->HasChanges();

    _model->SetFaceInfo(_facesPanel->GetFaceInfo());
    _model->SetStateInfo(_statesPanel->GetStateInfo());
    _subModelsPanel->Save();

    _contentChanged = facesChanged || statesChanged || subModelsChanged;
}

void ModelDefinitionsDialog::OnNotebookPageChanged(wxBookCtrlEvent& event)
{
    if (event.GetEventObject() != _notebook) {
        event.Skip();
        return;
    }

    int oldPage = event.GetOldSelection();
    int newPage = event.GetSelection();

    if (oldPage != wxNOT_FOUND) {
        switch (oldPage) {
            case TAB_FACES:     _facesPanel->OnDeactivate();     break;
            case TAB_STATES:    _statesPanel->OnDeactivate();    break;
            case TAB_SUBMODELS: _subModelsPanel->OnDeactivate(); break;
        }
    }

    switch (newPage) {
        case TAB_FACES:
            // Refresh the state outline dropdown from the States panel's live in-memory data
            _facesPanel->RefreshStateOutlineChoice(_statesPanel->GetStateInfo());
            _facesPanel->OnActivate();
            break;
        case TAB_STATES:    _statesPanel->OnActivate();    break;
        case TAB_SUBMODELS: _subModelsPanel->OnActivate(); break;
    }

    event.Skip();
}

void ModelDefinitionsDialog::OnOK(wxCommandEvent&)
{
    if (_statesPanel->HasInvalidRows()) {
        wxMessageBox("One or more state rows have a name without a channel or a channel without a name. Please fix the highlighted rows before saving.",
                     "Invalid State Data", wxOK | wxICON_WARNING, this);
        _notebook->SetSelection(TAB_STATES);
        return;
    }
    CommitAllChanges();
    EndModal(wxID_OK);
}

void ModelDefinitionsDialog::ConfirmClose()
{
    if (wxMessageBox("Are you sure you want to close?",
                     "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    EndModal(wxID_CANCEL);
}

void ModelDefinitionsDialog::OnClose(wxCloseEvent&)          { ConfirmClose(); }
void ModelDefinitionsDialog::OnCancelButton(wxCommandEvent&)  { ConfirmClose(); }
